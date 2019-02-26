/**
 * \file oqs_cpp.h
 * \brief Main header file for the liboqs C++ wrapper
 */

#ifndef OQS_CPP_H_
#define OQS_CPP_H_

// everything in liboqs has C linkage
extern "C" {
#include <oqs/oqs.h>
}

#include <algorithm>
#include <cstdint>
#include <cstdlib>
#include <cstring>
#include <exception>
#include <memory>
#include <ostream>
#include <string>
#include <utility>
#include <vector>

/**
 * \namespace oqs
 * \brief Main namespace for the liboqs C++ wrapper
 */
namespace oqs {

using byte = std::uint8_t;       ///< byte (unsigned)
using bytes = std::vector<byte>; ///< vector of bytes (unsigned)

/**
 * \namespace impl_details
 * \brief Implementation details
 */
namespace impl_details_ {
/* code from
https://github.com/vsoftco/qpp/blob/master/include/internal/classes/singleton.h
 */
/**
 * \brief Singleton class using CRTP pattern
 * \tparam T Class type of which instance will become a Singleton
 */
template <typename T>
class Singleton {
  protected:
    Singleton() noexcept = default;

    Singleton(const Singleton&) = delete;

    Singleton& operator=(const Singleton&) = delete;

    virtual ~Singleton() = default;

  public:
    /**
     * \brief Singleton instance (thread-safe) via CRTP pattern
     * \note Code from
     * https://github.com/vsoftco/qpp/blob/master/include/internal/classes/singleton.h
     * \return Singleton instance
     */
    static T& get_instance() noexcept(std::is_nothrow_constructible<T>::value) {
        // Guaranteed to be destroyed.
        // Instantiated on first use.
        // Thread safe in C++11
        static T instance;

        return instance;
    }
}; // class Singleton
} // namespace impl_details_

/**
 * \class MechanismNotSupportedError
 * \brief Cryptographic scheme not supported
 */
class MechanismNotSupportedError : public std::runtime_error {
  public:
    /**
     * \brief Constructor
     * \param alg_name Cryptographic algorithm name
     */
    MechanismNotSupportedError(const std::string& alg_name)
        : std::runtime_error{alg_name + " is not supported by OQS"} {}
}; // class MechanismNotSupportedError

/**
 * \class MechanismNotEnabledError
 * \brief Cryptographic scheme not enabled
 */
class MechanismNotEnabledError : public std::runtime_error {
  public:
    /**
     * \brief Constructor
     * \param alg_name Cryptographic algorithm name
     */
    MechanismNotEnabledError(const std::string& alg_name)
        : std::runtime_error{alg_name + " is not enabled by OQS"} {}
}; // class MechanismNotEnabledError

/**
 * \class KEMs
 * \brief Singleton class, contains details about supported/enabled key exchange
 * mechanisms (KEMs)
 */
class KEMs final : public impl_details_::Singleton<const KEMs> {
    friend class impl_details_::Singleton<const KEMs>;
    /**
     * \brief Private default constructor
     * \note Use oqs::KEMs::get_instance() to create an instance
     */
    KEMs() = default;

  public:
    /**
     * \brief Maximum number of supported KEMs
     * \return Maximum number of supported KEMs
     */
    static std::size_t max_number_KEMs() {
        static std::size_t max_number_KEMs_ = ::OQS_KEM_alg_count();
        return max_number_KEMs_;
    }

    /**
     * \brief Checks whether the KEM algorithm \a alg_name is supported
     * \param alg_name Cryptographic algorithm name
     * \return True if the KEM algorithm is supported, false otherwise
     */
    static bool is_KEM_supported(const std::string& alg_name) {
        auto supported_KEMs = get_supported_KEMs();

        return std::find(supported_KEMs.begin(), supported_KEMs.end(),
                         alg_name) != supported_KEMs.end();
    }

    /**
     * \brief Checks whether the KEM algorithm \a alg_name is enabled
     * \param alg_name Cryptographic algorithm name
     * \return True if the KEM algorithm is enabled, false otherwise
     */
    static bool is_KEM_enabled(const std::string& alg_name) {
        ::OQS_KEM* kem = ::OQS_KEM_new(alg_name.c_str());
        if (kem) {
            ::OQS_KEM_free(kem);
            return true;
        }

        return false;
    }

    /**
     * \brief KEM algorithm name
     * \param alg_id Cryptographic algorithm numerical id
     * \return KEM algorithm name
     */
    static std::string get_KEM_name(std::size_t alg_id) {
        if (alg_id >= max_number_KEMs())
            throw std::out_of_range("Algorithm ID out of range");

        return ::OQS_KEM_alg_identifier(alg_id);
    }

    /**
     * \brief List of supported KEM algorithms
     * \return List of supported KEM algorithms
     */
    static std::vector<std::string> get_supported_KEMs() {
        std::vector<std::string> supported_KEMs;
        for (std::size_t i = 0; i < max_number_KEMs(); ++i)
            supported_KEMs.emplace_back(get_KEM_name(i));

        return supported_KEMs;
    }

    /**
     * \brief List of enabled KEM algorithms
     * \return List of enabled KEM algorithms
     */
    static std::vector<std::string> get_enabled_KEMs() {
        std::vector<std::string> enabled_KEMs;
        for (std::size_t i = 0;
             i < max_number_KEMs() && is_KEM_enabled(get_KEM_name(i)); ++i)
            enabled_KEMs.emplace_back(get_KEM_name(i));

        return enabled_KEMs;
    }
}; // class KEMs

/**
 * \class KeyEncapsulation
 * \brief Key encapsulation mechanisms
 */
class KeyEncapsulation {
    const std::string alg_name_; ///< cryptographic algorithm name
    std::shared_ptr<::OQS_KEM> kem_{nullptr, [](::OQS_KEM* p) {
                                        ::OQS_KEM_free(p);
                                    }}; ///< liboqs smart pointer to ::OQS_KEM
    bytes secret_key_{};                ///< secret key

    /**
     * \brief KEM algorithm details
     */
    struct alg_details_ {
        std::string name;
        std::string version;
        std::size_t claimed_nist_level;
        bool is_ind_cca;
        std::size_t length_public_key;
        std::size_t length_secret_key;
        std::size_t length_ciphertext;
        std::size_t length_shared_secret;
    } details_{};

  public:
    /**
     * \brief Constructs an instance of oqs::KeyEncapsulation
     * \param alg_name Cryptographic algorithm name
     * \param secret_key Secret key (optional)
     */
    KeyEncapsulation(const std::string& alg_name, const bytes& secret_key = {})
        : alg_name_{alg_name} {
        // KEM not enabled
        if (std::find(KEMs::get_enabled_KEMs().begin(),
                      KEMs::get_enabled_KEMs().end(),
                      alg_name) == KEMs::get_enabled_KEMs().end()) {
            // perhaps it's supported
            if (std::find(KEMs::get_supported_KEMs().begin(),
                          KEMs::get_supported_KEMs().end(),
                          alg_name) != KEMs::get_supported_KEMs().end())
                throw MechanismNotEnabledError(alg_name);
            else
                throw MechanismNotSupportedError(alg_name);
        }

        kem_.reset(::OQS_KEM_new(alg_name.c_str()));

        details_.name = kem_->method_name;
        details_.version = kem_->alg_version;
        details_.claimed_nist_level = kem_->claimed_nist_level;
        details_.is_ind_cca = kem_->ind_cca;
        details_.length_public_key = kem_->length_public_key;
        details_.length_secret_key = kem_->length_secret_key;
        details_.length_ciphertext = kem_->length_ciphertext;
        details_.length_shared_secret = kem_->length_shared_secret;

        if (!secret_key.empty()) {
            secret_key_ = secret_key;
        }
    }

    /**
     * \brief Virtual default destructor
     */
    virtual ~KeyEncapsulation() {
        if (!secret_key_.empty())
            ::OQS_MEM_cleanse(secret_key_.data(), secret_key_.size());
    }

    /**
     * \brief KEM algorithm details
     * \return KEM algorithm details
     */
    const alg_details_& get_details() const { return details_; }

    /**
     * \brief Generate public key
     * \return Public key
     */
    bytes generate_keypair() {
        bytes public_key(get_details().length_public_key, 0);
        secret_key_ = bytes(get_details().length_secret_key, 0);

        ::OQS_STATUS rv_ = ::OQS_KEM_keypair(kem_.get(), public_key.data(),
                                             secret_key_.data());
        if (rv_ != ::OQS_SUCCESS)
            throw std::runtime_error("Can not generate keypair");

        return public_key;
    }

    /**
     * \brief Export secret key
     * \return Secret key
     */
    bytes export_secret_key() const { return secret_key_; }

    /**
     * \brief Encapsulate secret
     * \param public_key Public key
     * \return Pair consisting of 1) ciphertext, and 2) shared secret
     */
    std::pair<bytes, bytes> encap_secret(const bytes& public_key) const {
        bytes ciphertext(get_details().length_ciphertext, 0);
        bytes shared_secret(get_details().length_shared_secret, 0);
        ::OQS_STATUS rv_ =
            ::OQS_KEM_encaps(kem_.get(), ciphertext.data(),
                             shared_secret.data(), public_key.data());
        if (rv_ != ::OQS_SUCCESS)
            throw std::runtime_error("Can not encapsulate secret");

        return std::make_pair(ciphertext, shared_secret);
    }

    /**
     * \brief Decapsulate secret
     * \param ciphertext Ciphertext
     * \return Shared secret
     */
    bytes decap_secret(const bytes& ciphertext) const {
        bytes shared_secret(get_details().length_shared_secret, 0);
        ::OQS_STATUS rv_ =
            ::OQS_KEM_decaps(kem_.get(), shared_secret.data(),
                             ciphertext.data(), secret_key_.data());

        if (rv_ != ::OQS_SUCCESS)
            throw std::runtime_error("Can not decapsulate secret");

        return shared_secret;
    }

    /**
     * \brief std::ostream extraction operator for the KEM algorithm details
     * \param os Output stream
     * \param rhs Algorithm details instance
     * \return Reference to the output stream
     */
    friend std::ostream& operator<<(std::ostream& os, const alg_details_& rhs) {
        os << "Name: " << rhs.name << '\n';
        os << "Version: " << rhs.version << '\n';
        os << "Claimed NIST level: " << rhs.claimed_nist_level << '\n';
        os << "Is IND_CCA: " << rhs.is_ind_cca << '\n';
        os << "Length public key (bytes): " << rhs.length_public_key << '\n';
        os << "Length secret key (bytes): " << rhs.length_secret_key << '\n';
        os << "Length ciphertext (bytes): " << rhs.length_ciphertext << '\n';
        os << "Length shared secret (bytes): " << rhs.length_shared_secret;
        return os;
    }

    /**
     * \brief std::ostream extraction operator for oqs::KeyEncapsulation
     * \param os Output stream
     * \param rhs Key encapsulation instance
     * \return Reference to the output stream
     */
    friend std::ostream& operator<<(std::ostream& os,
                                    const KeyEncapsulation& rhs) {
        return os << "Key encapsulation mechanism: " << rhs.get_details().name;
    }
}; // class KeyEncapsulation

/**
 * \class Sigs
 * \brief Singleton class, contains details about supported/enabled signature
 * mechanisms
 */
class Sigs final : public impl_details_::Singleton<const Sigs> {
    friend class impl_details_::Singleton<const Sigs>;
    /**
     * \brief Private default constructor
     * \note Use oqs::Sigs::get_instance() to create an instance
     */
    Sigs() = default;

  public:
    /**
     * \brief Maximum number of supported signatures
     * \return Maximum number of supported signatures
     */
    static std::size_t max_number_Sigs() {
        static std::size_t max_number_Sigs_ = ::OQS_SIG_alg_count();
        return max_number_Sigs_;
    }

    /**
     * \brief Checks whether the signature algorithm \a alg_name is supported
     * \param alg_name Cryptographic algorithm name
     * \return True if the signature algorithm is supported, false otherwise
     */
    static bool is_Sig_supported(const std::string& alg_name) {
        auto supported_Sigs = get_supported_Sigs();

        return std::find(supported_Sigs.begin(), supported_Sigs.end(),
                         alg_name) != supported_Sigs.end();
    }

    /**
     * \brief Checks whether the signature algorithm \a alg_name is enabled
     * \param alg_name Cryptographic algorithm name
     * \return True if the signature algorithm is enabled, false otherwise
     */
    static bool is_Sig_enabled(const std::string& alg_name) {
        ::OQS_SIG* sig = ::OQS_SIG_new(alg_name.c_str());
        if (sig) {
            ::OQS_SIG_free(sig);
            return true;
        }

        return false;
    }

    /**
     * \brief Signature algorithm name
     * \param alg_id Cryptographic algorithm numerical id
     * \return Signature algorithm name
     */
    static std::string get_Sig_name(std::size_t alg_id) {
        if (alg_id >= max_number_Sigs())
            throw std::out_of_range("Algorithm ID out of range");

        return ::OQS_SIG_alg_identifier(alg_id);
    }

    /**
     * \brief List of supported signature algorithms
     * \return List of supported signature algorithms
     */
    static std::vector<std::string> get_supported_Sigs() {
        std::vector<std::string> supported_Sigs;
        for (std::size_t i = 0; i < max_number_Sigs(); ++i)
            supported_Sigs.emplace_back(get_Sig_name(i));

        return supported_Sigs;
    }

    /**
     * \brief List of enabled KEM algorithms
     * \return List of enabled KEM algorithms
     */
    static std::vector<std::string> get_enabled_Sigs() {
        std::vector<std::string> enabled_Sigs;
        for (std::size_t i = 0;
             i < max_number_Sigs() && is_Sig_enabled(get_Sig_name(i)); ++i)
            enabled_Sigs.emplace_back(get_Sig_name(i));

        return enabled_Sigs;
    }
}; // class Sigs

/**
 * \class Signature
 * \brief Signature mechanisms
 */
class Signature {
    const std::string alg_name_; ///< cryptographic algorithm name
    std::shared_ptr<::OQS_SIG> sig_{nullptr, [](::OQS_SIG* p) {
                                        ::OQS_SIG_free(p);
                                    }}; ///< liboqs smart pointer to ::OQS_SIG
    bytes secret_key_{};                ///< secret key

    /**
     * \brief Signature algorithm details
     */
    struct alg_details_ {
        std::string name;
        std::string version;
        std::size_t claimed_nist_level;
        bool is_euf_cma;
        std::size_t length_public_key;
        std::size_t length_secret_key;
        std::size_t length_signature;
    } details_{};

  public:
    /**
     * \brief Constructs an instance of oqs::Signature
     * \param alg_name Cryptographic algorithm name
     * \param secret_key Secret key (optional)
     */
    Signature(const std::string& alg_name, const bytes& secret_key = {})
        : alg_name_{alg_name} {
        // Sig not enabled
        if (std::find(Sigs::get_enabled_Sigs().begin(),
                      Sigs::get_enabled_Sigs().end(),
                      alg_name) == Sigs::get_enabled_Sigs().end()) {
            // perhaps it's supported
            if (std::find(Sigs::get_supported_Sigs().begin(),
                          Sigs::get_supported_Sigs().end(),
                          alg_name) != Sigs::get_supported_Sigs().end())
                throw MechanismNotEnabledError(alg_name);
            else
                throw MechanismNotSupportedError(alg_name);
        }

        sig_.reset(::OQS_SIG_new(alg_name.c_str()));

        details_.name = sig_->method_name;
        details_.version = sig_->alg_version;
        details_.claimed_nist_level = sig_->claimed_nist_level;
        details_.is_euf_cma = sig_->euf_cma;
        details_.length_public_key = sig_->length_public_key;
        details_.length_secret_key = sig_->length_secret_key;
        details_.length_signature = sig_->length_signature;

        if (!secret_key.empty()) {
            secret_key_ = secret_key;
        }
    }

    /**
     * \brief Virtual default destructor
     */
    virtual ~Signature() {
        if (!secret_key_.empty())
            ::OQS_MEM_cleanse(secret_key_.data(), secret_key_.size());
    }

    /**
     * \brief Signature algorithm details
     * \return Signature algorithm details
     */
    const alg_details_& get_details() const { return details_; }

    /**
     * \brief Generate public key
     * \return Public key
     */
    bytes generate_keypair() {
        bytes public_key(get_details().length_public_key, 0);
        secret_key_ = bytes(get_details().length_secret_key, 0);

        ::OQS_STATUS rv_ = ::OQS_SIG_keypair(sig_.get(), public_key.data(),
                                             secret_key_.data());
        if (rv_ != ::OQS_SUCCESS)
            throw std::runtime_error("Can not generate keypair");

        return public_key;
    }

    /**
     * \brief Export secret key
     * \return Secret key
     */
    bytes export_secret_key() const { return secret_key_; }

    /**
     * \brief Sign message
     * \param message Message
     * \return Message signature
     */
    bytes sign(const bytes& message) {
        bytes signature(get_details().length_signature, 0);
        std::size_t sig_len = 0;
        ::OQS_STATUS rv_ =
            ::OQS_SIG_sign(sig_.get(), signature.data(), &sig_len,
                           message.data(), message.size(), secret_key_.data());

        if (rv_ != ::OQS_SUCCESS)
            throw std::runtime_error("Can not sign message");

        signature.resize(sig_len);

        return signature;
    }

    /**
     * \brief Verify signature
     * \param message Message
     * \param signature Signature
     * \param public_key Public key
     * \return True if the signature is valid, false otherwise
     */
    bool verify(const bytes& message, const bytes& signature,
                const bytes& public_key) {
        ::OQS_STATUS rv_ = ::OQS_SIG_verify(
            sig_.get(), message.data(), message.size(), signature.data(),
            signature.size(), public_key.data());

        if (rv_ != ::OQS_SUCCESS)
            throw std::runtime_error("Can not verify signature");

        return true;
    }

    /**
     * \brief std::ostream extraction operator for the signature algorithm
     * details
     * \param os Output stream
     * \param rhs Algorithm details
     * \return Reference to the output stream
     */
    friend std::ostream& operator<<(std::ostream& os, const alg_details_& rhs) {
        os << "Name: " << rhs.name << '\n';
        os << "Version: " << rhs.version << '\n';
        os << "Claimed NIST level: " << rhs.claimed_nist_level << '\n';
        os << "Is EUF_CMA: " << rhs.is_euf_cma << '\n';
        os << "Length public key (bytes): " << rhs.length_public_key << '\n';
        os << "Length secret key (bytes): " << rhs.length_secret_key << '\n';
        os << "Length signature (bytes): " << rhs.length_signature;
        return os;
    }

    /**
     * \brief std::ostream extraction operator for oqs::Signature
     * \param os Output stream
     * \param rhs Signature instance
     * \return Reference to the output stream
     */
    friend std::ostream& operator<<(std::ostream& os, const Signature& rhs) {
        return os << "Signature mechanism: " << rhs.get_details().name;
    }
}; // class Signature

namespace impl_details_ {
// initialize the KEMs and Sigs singletons
static const KEMs& algs_ =
    KEMs::get_instance(); ///< initializes the KEMs singleton
static const Sigs& sigs_ =
    Sigs::get_instance(); ///< initializes the Sigs singleton
} // namespace impl_details_
} // namespace oqs

/**
 * \std::ostream extraction operator for oqs::bytes
 * \param os Output stream
 * \param rhs Signature instance
 * \return Reference to the output stream
 */
inline std::ostream& operator<<(std::ostream& os, const oqs::bytes& rhs) {
    bool first = true;
    for (auto&& elem : rhs) {
        if (first) {
            first = false;
            os << std::hex << std::uppercase << static_cast<int>(elem);
        } else {
            os << " " << std::hex << std::uppercase << static_cast<int>(elem);
        }
    }

    return os;
}

/**
 * \std::ostream extraction operator for vectors of strings
 * \param os Output stream
 * \param rhs Signature instance
 * \return Reference to the output stream
 */
inline std::ostream& operator<<(std::ostream& os,
                                const std::vector<std::string>& rhs) {
    bool first = true;
    for (auto&& elem : rhs) {
        if (first) {
            first = false;
            os << elem;
        } else {
            os << " " << elem;
        }
    }

    return os;
}

inline namespace oqs_literals {
/**
 * \brief User-defined literal operator for converting C-style strings to
 * oqs::bytes
 * \note The null terminator is not included
 * \param c_str C-style string
 * \param length C-style string length (deduced automatically by the compiler)
 * \return The byte representation of the input C-style string
 */
inline oqs::bytes operator""_bytes(const char* c_str, std::size_t length) {
    oqs::bytes result(length);
    for (std::size_t i = 0; i < length; ++i)
        result[i] = static_cast<uint8_t>(c_str[i]);

    return result;
}
} // namespace oqs_literals

#endif // OQS_CPP_H_
