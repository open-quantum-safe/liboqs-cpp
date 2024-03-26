/**
 * \file oqs_cpp.hpp
 * \brief Main header file for the liboqs C++ wrapper
 */

#ifndef OQS_CPP_HPP_
#define OQS_CPP_HPP_

#include <algorithm>
#include <cstdlib>
#include <iostream>
#include <memory>
#include <ostream>
#include <stdexcept>
#include <string>
#include <utility>
#include <vector>

#include "common.hpp"

/**
 * \namespace oqs
 * \brief Main namespace for the liboqs C++ wrapper
 */
namespace oqs {
/**
 * \namespace oqs::C
 * \brief Namespace containing all of the oqs C functions, so they do not
 * pollute the oqs namespace
 */
namespace C {
// Everything in liboqs has C linkage
extern "C" {
#include <oqs/oqs.h>
}
} // namespace C

/**
 * \class oqs::MechanismNotSupportedError
 * \brief Cryptographic scheme not supported
 */
class MechanismNotSupportedError : public std::runtime_error {
  public:
    /**
     * \brief Constructor
     * \param alg_name Cryptographic algorithm name
     */
    explicit MechanismNotSupportedError(const std::string& alg_name)
        : std::runtime_error{"\"" + alg_name + "\"" +
                             " is not supported by OQS"} {}
}; // class MechanismNotSupportedError

/**
 * \class oqs::MechanismNotEnabledError
 * \brief Cryptographic scheme not enabled
 */
class MechanismNotEnabledError : public std::runtime_error {
  public:
    /**
     * \brief Constructor
     * \param alg_name Cryptographic algorithm name
     */
    explicit MechanismNotEnabledError(const std::string& alg_name)
        : std::runtime_error{"\"" + alg_name + "\"" +
                             " is not enabled by OQS"} {}
}; // class MechanismNotEnabledError

/**
 * \class oqs::KEMs
 * \brief Singleton class, contains details about supported/enabled key exchange
 * mechanisms (KEMs)
 */
class KEMs final : public internal::Singleton<const KEMs> {
    friend class internal::Singleton<const KEMs>;

    /**
     * \brief Private default constructor
     * \note Use oqs::KEMs::get_instance() to create an instance
     */
    KEMs() = default;

  public:
    /**
     * \brief Maximum number of supported KEM algorithms
     * \return Maximum number of supported KEM algorithms
     */
    static std::size_t max_number_KEMs() {
        static std::size_t max_number_KEMs_ = C::OQS_KEM_alg_count();

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
        return C::OQS_KEM_alg_is_enabled(alg_name.c_str());
    }

    /**
     * \brief KEM algorithm name
     * \param alg_id Cryptographic algorithm numerical id
     * \return KEM algorithm name
     */
    static std::string get_KEM_name(std::size_t alg_id) {
        if (alg_id >= max_number_KEMs())
            throw std::out_of_range("Algorithm ID out of range");

        return C::OQS_KEM_alg_identifier(alg_id);
    }

    /**
     * \brief Vector of supported KEM algorithms
     * \return Vector of supported KEM algorithms
     */
    static const std::vector<std::string>& get_supported_KEMs() {
        static std::vector<std::string> supported_KEMs;
        static bool fnc_already_invoked = false; // function was already invoked

        if (!fnc_already_invoked) {
            for (std::size_t i = 0; i < max_number_KEMs(); ++i)
                supported_KEMs.emplace_back(get_KEM_name(i));
            fnc_already_invoked = true;
        }

        return supported_KEMs;
    }

    /**
     * \brief Vector of enabled KEM algorithms
     * \return Vector of enabled KEM algorithms
     */
    static const std::vector<std::string>& get_enabled_KEMs() {
        static std::vector<std::string> enabled_KEMs;
        static bool fnc_already_invoked = false; // function was already invoked

        if (!fnc_already_invoked) {
            for (auto&& elem : get_supported_KEMs())
                if (is_KEM_enabled(elem))
                    enabled_KEMs.emplace_back(elem);
            fnc_already_invoked = true;
        }

        return enabled_KEMs;
    }
}; // class KEMs

/**
 * \class oqs::KeyEncapsulation
 * \brief Key encapsulation mechanisms
 */
class KeyEncapsulation {
    std::shared_ptr<C::OQS_KEM> kem_{nullptr, [](C::OQS_KEM* p) {
                                         C::OQS_KEM_free(p);
                                     }}; ///< liboqs smart pointer to C::OQS_KEM
    bytes secret_key_{};                 ///< secret key
  public:
    /**
     * \brief KEM algorithm details
     */
    struct KeyEncapsulationDetails {
        std::string name;
        std::string version;
        std::size_t claimed_nist_level;
        bool is_ind_cca;
        std::size_t length_public_key;
        std::size_t length_secret_key;
        std::size_t length_ciphertext;
        std::size_t length_shared_secret;
    };

  private:
    KeyEncapsulationDetails alg_details_{}; ///< KEM algorithm details

  public:
    /**
     * \brief Constructs an instance of oqs::KeyEncapsulation
     * \param alg_name Cryptographic algorithm name
     * \param secret_key Secret key (optional)
     */
    explicit KeyEncapsulation(const std::string& alg_name,
                              bytes secret_key = {})
        : secret_key_{std::move(secret_key)} {
        // KEM not enabled
        if (!KEMs::is_KEM_enabled(alg_name)) {
            // Perhaps it's supported
            if (KEMs::is_KEM_supported(alg_name))
                throw MechanismNotEnabledError(alg_name);
            else
                throw MechanismNotSupportedError(alg_name);
        }

        kem_.reset(C::OQS_KEM_new(alg_name.c_str()),
                   [](C::OQS_KEM* p) { C::OQS_KEM_free(p); });

        alg_details_.name = kem_->method_name;
        alg_details_.version = kem_->alg_version;
        alg_details_.claimed_nist_level = kem_->claimed_nist_level;
        alg_details_.is_ind_cca = kem_->ind_cca;
        alg_details_.length_public_key = kem_->length_public_key;
        alg_details_.length_secret_key = kem_->length_secret_key;
        alg_details_.length_ciphertext = kem_->length_ciphertext;
        alg_details_.length_shared_secret = kem_->length_shared_secret;
    }

    /**
     * \brief Default copy constructor
     */
    KeyEncapsulation(const KeyEncapsulation&) = default;

    /**
     * \brief Default copy assignment operator
     * \return Reference to the current instance
     */
    KeyEncapsulation& operator=(const KeyEncapsulation&) = default;

    /**
     * \brief Move constructor, guarantees that the rvalue secret key is always
     * zeroed
     * \param rhs oqs::KeyEncapsulation instance
     */
    KeyEncapsulation(KeyEncapsulation&& rhs) noexcept
        : kem_{std::move(rhs.kem_)}, alg_details_{std::move(rhs.alg_details_)} {
        // Paranoid move via copy/clean/resize, see
        // https://stackoverflow.com/questions/55054187/can-i-resize-a-vector-that-was-moved-from
        secret_key_ = rhs.secret_key_; // copy
        // Clean (zero)
        C::OQS_MEM_cleanse(rhs.secret_key_.data(), rhs.secret_key_.size());
        rhs.secret_key_.resize(0); // resize
    }
    /**
     * \brief Move assignment operator, guarantees that the rvalue secret key is
     * always zeroed
     * \param rhs oqs::KeyEncapsulation instance
     * \return Reference to the current instance
     */
    KeyEncapsulation& operator=(KeyEncapsulation&& rhs) noexcept {
        kem_ = std::move(rhs.kem_);
        alg_details_ = std::move(rhs.alg_details_);

        // Paranoid move via copy/clean/resize, see
        // https://stackoverflow.com/questions/55054187/can-i-resize-a-vector-that-was-moved-from
        secret_key_ = rhs.secret_key_; // copy
        // Clean (zero)
        C::OQS_MEM_cleanse(rhs.secret_key_.data(), rhs.secret_key_.size());
        rhs.secret_key_.resize(0); // resize

        return *this;
    }

    /**
     * \brief Virtual default destructor
     */
    virtual ~KeyEncapsulation() {
        if (!secret_key_.empty())
            C::OQS_MEM_cleanse(secret_key_.data(), secret_key_.size());
    }

    /**
     * \brief KEM algorithm details, lvalue overload
     * \return KEM algorithm details
     */
    const KeyEncapsulationDetails& get_details() const& { return alg_details_; }

    /**
     * \brief KEM algorithm details, rvalue overload
     * \return KEM algorithm details
     */
    KeyEncapsulationDetails get_details() const&& { return alg_details_; }

    /**
     * \brief Generate public key/secret key pair
     * \return Public key
     */
    bytes generate_keypair() {
        bytes public_key(alg_details_.length_public_key, 0);
        secret_key_ = bytes(alg_details_.length_secret_key, 0);

        OQS_STATUS rv_ = C::OQS_KEM_keypair(kem_.get(), public_key.data(),
                                            secret_key_.data());
        if (rv_ != OQS_STATUS::OQS_SUCCESS)
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
        if (public_key.size() != alg_details_.length_public_key)
            throw std::runtime_error("Incorrect public key length");

        bytes ciphertext(alg_details_.length_ciphertext, 0);
        bytes shared_secret(alg_details_.length_shared_secret, 0);
        OQS_STATUS rv_ =
            C::OQS_KEM_encaps(kem_.get(), ciphertext.data(),
                              shared_secret.data(), public_key.data());
        if (rv_ != OQS_STATUS::OQS_SUCCESS)
            throw std::runtime_error("Can not encapsulate secret");

        return std::make_pair(ciphertext, shared_secret);
    }

    /**
     * \brief Decapsulate secret
     * \param ciphertext Ciphertext
     * \return Shared secret
     */
    bytes decap_secret(const bytes& ciphertext) const {
        if (ciphertext.size() != alg_details_.length_ciphertext)
            throw std::runtime_error("Incorrect ciphertext length");

        if (secret_key_.size() != alg_details_.length_secret_key)
            throw std::runtime_error(
                "Incorrect secret key length, make sure you "
                "specify one in the constructor or run "
                "oqs::Signature::generate_keypair()");

        bytes shared_secret(alg_details_.length_shared_secret, 0);
        OQS_STATUS rv_ =
            C::OQS_KEM_decaps(kem_.get(), shared_secret.data(),
                              ciphertext.data(), secret_key_.data());

        if (rv_ != OQS_STATUS::OQS_SUCCESS)
            throw std::runtime_error("Can not decapsulate secret");

        return shared_secret;
    }

    /**
     * \brief std::ostream extraction operator for the KEM algorithm details
     * \param os Output stream
     * \param rhs Algorithm details instance
     * \return Reference to the output stream
     */
    friend std::ostream& operator<<(std::ostream& os,
                                    const KeyEncapsulationDetails& rhs) {
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
     * \param rhs oqs::KeyEncapsulation instance
     * \return Reference to the output stream
     */
    friend std::ostream& operator<<(std::ostream& os,
                                    const KeyEncapsulation& rhs) {
        return os << "Key encapsulation mechanism: " << rhs.alg_details_.name;
    }
}; // class KeyEncapsulation

/**
 * \class oqs::Sigs
 * \brief Singleton class, contains details about supported/enabled signature
 * mechanisms
 */
class Sigs final : public internal::Singleton<const Sigs> {
    friend class internal::Singleton<const Sigs>;

    /**
     * \brief Private default constructor
     * \note Use oqs::Sigs::get_instance() to create an instance
     */
    Sigs() = default;

  public:
    /**
     * \brief Maximum number of supported signature algorithms
     * \return Maximum number of supported signature algorithms
     */
    static std::size_t max_number_sigs() {
        static std::size_t max_number_sigs_ = C::OQS_SIG_alg_count();

        return max_number_sigs_;
    }

    /**
     * \brief Checks whether the signature algorithm \a alg_name is supported
     * \param alg_name Cryptographic algorithm name
     * \return True if the signature algorithm is supported, false otherwise
     */
    static bool is_sig_supported(const std::string& alg_name) {
        auto supported_sigs = get_supported_sigs();

        return std::find(supported_sigs.begin(), supported_sigs.end(),
                         alg_name) != supported_sigs.end();
    }

    /**
     * \brief Checks whether the signature algorithm \a alg_name is enabled
     * \param alg_name Cryptographic algorithm name
     * \return True if the signature algorithm is enabled, false otherwise
     */
    static bool is_sig_enabled(const std::string& alg_name) {
        return C::OQS_SIG_alg_is_enabled(alg_name.c_str());
    }

    /**
     * \brief Signature algorithm name
     * \param alg_id Cryptographic algorithm numerical id
     * \return Signature algorithm name
     */
    static std::string get_sig_name(std::size_t alg_id) {
        if (alg_id >= max_number_sigs())
            throw std::out_of_range("Algorithm ID out of range");

        return C::OQS_SIG_alg_identifier(alg_id);
    }

    /**
     * \brief Vector of supported signature algorithms
     * \return Vector of supported signature algorithms
     */
    static const std::vector<std::string>& get_supported_sigs() {
        static std::vector<std::string> supported_sigs;
        static bool fnc_already_invoked = false;

        if (!fnc_already_invoked) {
            for (std::size_t i = 0; i < max_number_sigs(); ++i)
                supported_sigs.emplace_back(get_sig_name(i));
            fnc_already_invoked = true;
        }

        return supported_sigs;
    }

    /**
     * \brief Vector of enabled signature algorithms
     * \return Vector of enabled signature algorithms
     */
    static const std::vector<std::string>& get_enabled_sigs() {
        static std::vector<std::string> enabled_sigs;
        static bool fnc_already_invoked = false;

        if (!fnc_already_invoked) {
            for (auto&& elem : get_supported_sigs())
                if (is_sig_enabled(elem))
                    enabled_sigs.emplace_back(elem);
            fnc_already_invoked = true;
        }

        return enabled_sigs;
    }
}; // class Sigs

/**
 * \class oqs::Signature
 * \brief Signature mechanisms
 */
class Signature {
    std::shared_ptr<C::OQS_SIG> sig_{nullptr, [](C::OQS_SIG* p) {
                                         C::OQS_SIG_free(p);
                                     }}; ///< liboqs smart pointer to C::OQS_SIG
    bytes secret_key_{};                 ///< secret key

  public:
    /**
     * \brief Signature algorithm details
     */
    struct SignatureDetails {
        std::string name;
        std::string version;
        std::size_t claimed_nist_level;
        bool is_euf_cma;
        std::size_t length_public_key;
        std::size_t length_secret_key;
        std::size_t max_length_signature;
    };

  private:
    SignatureDetails alg_details_{}; ///< Signature algorithm details

  public:
    /**
     * \brief Constructs an instance of oqs::Signature
     * \param alg_name Cryptographic algorithm name
     * \param secret_key Secret key (optional)
     */
    explicit Signature(const std::string& alg_name, bytes secret_key = {})
        : secret_key_{std::move(secret_key)} {
        // signature not enabled
        if (!Sigs::is_sig_enabled(alg_name)) {
            // perhaps it's supported
            if (Sigs::is_sig_supported(alg_name))
                throw MechanismNotEnabledError(alg_name);
            else
                throw MechanismNotSupportedError(alg_name);
        }

        sig_.reset(C::OQS_SIG_new(alg_name.c_str()),
                   [](C::OQS_SIG* p) { C::OQS_SIG_free(p); });

        alg_details_.name = sig_->method_name;
        alg_details_.version = sig_->alg_version;
        alg_details_.claimed_nist_level = sig_->claimed_nist_level;
        alg_details_.is_euf_cma = sig_->euf_cma;
        alg_details_.length_public_key = sig_->length_public_key;
        alg_details_.length_secret_key = sig_->length_secret_key;
        alg_details_.max_length_signature = sig_->length_signature;
    }

    /**
     * \brief Default copy constructor
     */
    Signature(const Signature&) = default;

    /**
     * \brief Default copy assignment operator
     * \return Reference to the current instance
     */
    Signature& operator=(const Signature&) = default;

    /**
     * \brief Move constructor, guarantees that the rvalue secret key is always
     * zeroed
     * \param rhs oqs::Signature instance
     */
    Signature(Signature&& rhs) noexcept
        : sig_{std::move(rhs.sig_)}, alg_details_{std::move(rhs.alg_details_)} {
        // Paranoid move via copy/clean/resize, see
        // https://stackoverflow.com/questions/55054187/can-i-resize-a-vector-that-was-moved-from
        secret_key_ = rhs.secret_key_; // copy
        // Clean (zero)
        C::OQS_MEM_cleanse(rhs.secret_key_.data(), rhs.secret_key_.size());
        rhs.secret_key_.resize(0); // resize
    }
    /**
     * \brief Move assignment operator, guarantees that the rvalue secret key is
     * always zeroed
     * \param rhs oqs::Signature instance
     * \return Reference to the current instance
     */
    Signature& operator=(Signature&& rhs) noexcept {
        sig_ = std::move(rhs.sig_);
        alg_details_ = std::move(rhs.alg_details_);

        // Paranoid move via copy/clean/resize, see
        // https://stackoverflow.com/questions/55054187/can-i-resize-a-vector-that-was-moved-from
        secret_key_ = rhs.secret_key_; // copy
        // Clean (zero)
        C::OQS_MEM_cleanse(rhs.secret_key_.data(), rhs.secret_key_.size());
        rhs.secret_key_.resize(0); // resize

        return *this;
    }

    /**
     * \brief Virtual default destructor
     */
    virtual ~Signature() {
        if (!secret_key_.empty())
            C::OQS_MEM_cleanse(secret_key_.data(), secret_key_.size());
    }

    /**
     * \brief Signature algorithm details, lvalue overload
     * \return Signature algorithm details
     */
    const SignatureDetails& get_details() const& { return alg_details_; }

    /**
     * \brief Signature algorithm details, rvalue overload
     * \return Signature algorithm details
     */
    SignatureDetails get_details() const&& { return alg_details_; }

    /**
     * \brief Generate public key/secret key pair
     * \return Public key
     */
    bytes generate_keypair() {
        bytes public_key(get_details().length_public_key, 0);
        secret_key_ = bytes(alg_details_.length_secret_key, 0);

        OQS_STATUS rv_ = C::OQS_SIG_keypair(sig_.get(), public_key.data(),
                                            secret_key_.data());
        if (rv_ != OQS_STATUS::OQS_SUCCESS)
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
    bytes sign(const bytes& message) const {
        if (secret_key_.size() != alg_details_.length_secret_key)
            throw std::runtime_error(
                "Incorrect secret key length, make sure you "
                "specify one in the constructor or run "
                "oqs::Signature::generate_keypair()");

        bytes signature(alg_details_.max_length_signature, 0);

        std::size_t len_sig;
        OQS_STATUS rv_ =
            C::OQS_SIG_sign(sig_.get(), signature.data(), &len_sig,
                            message.data(), message.size(), secret_key_.data());

        if (rv_ != OQS_STATUS::OQS_SUCCESS)
            throw std::runtime_error("Can not sign message");

        signature.resize(len_sig);

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
                const bytes& public_key) const {
        if (public_key.size() != alg_details_.length_public_key)
            throw std::runtime_error("Incorrect public key length");

        if (signature.size() > alg_details_.max_length_signature)
            throw std::runtime_error("Incorrect signature size");

        OQS_STATUS rv_ = C::OQS_SIG_verify(sig_.get(), message.data(),
                                           message.size(), signature.data(),
                                           signature.size(), public_key.data());

        return rv_ == OQS_STATUS::OQS_SUCCESS;
    }

    /**
     * \brief std::ostream extraction operator for the signature algorithm
     * details
     * \param os Output stream
     * \param rhs Algorithm details instance
     * \return Reference to the output stream
     */
    friend std::ostream& operator<<(std::ostream& os,
                                    const SignatureDetails& rhs) {
        os << "Name: " << rhs.name << '\n';
        os << "Version: " << rhs.version << '\n';
        os << "Claimed NIST level: " << rhs.claimed_nist_level << '\n';
        os << "Is EUF_CMA: " << rhs.is_euf_cma << '\n';
        os << "Length public key (bytes): " << rhs.length_public_key << '\n';
        os << "Length secret key (bytes): " << rhs.length_secret_key << '\n';
        os << "Maximum length signature (bytes): " << rhs.max_length_signature;

        return os;
    }

    /**
     * \brief std::ostream extraction operator for oqs::Signature
     * \param os Output stream
     * \param rhs oqs::Signature instance
     * \return Reference to the output stream
     */
    friend std::ostream& operator<<(std::ostream& os, const Signature& rhs) {
        return os << "Signature mechanism: " << rhs.alg_details_.name;
    }
}; // class Signature

namespace internal {
/**
 * \class oqs::internal::Init
 * \brief liboqs initialization
 */
class Init final : public internal::Singleton<const Init> {
    friend class internal::Singleton<const Init>;
    /**
     * \brief Private default constructor
     * \note Use oqs::internal::Init::get_instance() to create an instance
     */
    Init() {
        C::OQS_init();
        std::string oqs_ver = oqs_version();
        std::string oqs_cpp_ver = oqs_cpp_version();
        if (oqs_ver != oqs_cpp_ver) {
            std::cerr << "Warning! liboqs version " << oqs_ver
                      << " differs from liboqs-cpp version " << oqs_cpp_ver
                      << std::endl;
        }
    }
};
static const Init& init_ = Init::get_instance(); ///< liboqs initialization
// Instantiate the KEMs and Sigs singletons (if ever needed)
static const KEMs& kems_ =
    KEMs::get_instance(); ///< instantiates the KEMs singleton
static const Sigs& sigs_ =
    Sigs::get_instance(); ///< instantiates the Sigs singleton
} // namespace internal
} // namespace oqs

#endif // OQS_CPP_HPP_
