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
#include <ostream>
#include <string>
#include <utility>
#include <vector>

namespace oqs {

using bytes = std::vector<std::uint8_t>;

namespace impl_details_ {
/* code from
https://github.com/vsoftco/qpp/blob/master/include/internal/classes/singleton.h
 */
template <typename T>
class Singleton {
  protected:
    Singleton() noexcept = default;

    Singleton(const Singleton&) = delete;

    Singleton& operator=(const Singleton&) = delete;

    virtual ~Singleton() = default;

  public:
    static T& get_instance() noexcept(std::is_nothrow_constructible<T>::value) {
        // Guaranteed to be destroyed.
        // Instantiated on first use.
        // Thread safe in C++11
        static T instance;

        return instance;
    }
}; // class Singleton
} // namespace impl_details_

class MechanismNotSupportedError : public std::runtime_error {
  public:
    MechanismNotSupportedError(const std::string& alg_name)
        : std::runtime_error{alg_name + " is not supported by OQS"} {}
}; // class MechanismNotSupportedError

class MechanismNotEnabledError : public std::runtime_error {
  public:
    MechanismNotEnabledError(const std::string& alg_name)
        : std::runtime_error{alg_name + " is not enabled by OQS"} {}
}; // class MechanismNotEnabledError

class KEMs final : public impl_details_::Singleton<const KEMs> {
    static std::size_t max_number_KEMs_;
    static std::vector<std::string> supported_KEMs_;
    static std::vector<std::string> enabled_KEMs_;

  public:
    KEMs() {
        for (std::size_t i = 0; i < max_number_KEMs_; ++i) {
            std::string alg_name = ::OQS_KEM_alg_identifier(i);
            supported_KEMs_.emplace_back(alg_name);
            if (is_KEM_enabled(get_KEM_name(i)))
                enabled_KEMs_.emplace_back(alg_name);
        }
    }

    static std::string get_KEM_name(std::size_t alg_id) {
        if (alg_id >= max_number_KEMs_)
            throw std::out_of_range("Algorithm ID out of range");

        return ::OQS_KEM_alg_identifier(alg_id);
    }

    static bool is_KEM_enabled(const std::string& alg_name) {
        ::OQS_KEM* kem = ::OQS_KEM_new(alg_name.c_str());
        if (kem) {
            OQS_KEM_free(kem);
            return true;
        }

        return false;
    }

    static bool is_KEM_supported(const std::string& alg_name) {
        return std::find(supported_KEMs_.begin(), supported_KEMs_.end(),
                         alg_name) != supported_KEMs_.end();
    }

    static const std::vector<std::string>& get_enabled_KEMs() {
        return enabled_KEMs_;
    }

    static const std::vector<std::string>& get_supported_KEMs() {
        return supported_KEMs_;
    }
}; // KEMs

std::size_t KEMs::max_number_KEMs_ = ::OQS_KEM_alg_count();
std::vector<std::string> KEMs::supported_KEMs_;
std::vector<std::string> KEMs::enabled_KEMs_;

class KeyEncapsulation {
    const std::string alg_name_;
    ::OQS_KEM* kem_;
    bytes secret_key_;

    struct alg_details_ {
        std::string name;
        std::string version;
        std::size_t claimed_nist_level;
        bool is_ind_cca;
        std::size_t length_public_key;
        std::size_t length_secret_key;
        std::size_t length_ciphertext;
        std::size_t length_shared_secret;
    } details_;

  public:
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

        kem_ = ::OQS_KEM_new(alg_name.c_str());

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

    virtual ~KeyEncapsulation() {
        if (!secret_key_.empty())
            ::OQS_MEM_cleanse(secret_key_.data(), secret_key_.size());
        ::OQS_KEM_free(kem_);
    }

    const alg_details_& get_details() const { return details_; }

    bytes generate_keypair() {
        bytes public_key(get_details().length_public_key, 0);
        secret_key_ = bytes(get_details().length_secret_key, 0);

        ::OQS_STATUS rv_ =
            ::OQS_KEM_keypair(kem_, public_key.data(), secret_key_.data());
        if (rv_ != ::OQS_SUCCESS)
            throw std::runtime_error("Can not generate keypair");

        return public_key;
    }

    bytes export_secret_key() const { return secret_key_; }

    std::pair<bytes, bytes> encap_secret(const bytes& public_key) const {
        bytes ciphertext(get_details().length_ciphertext, 0);
        bytes shared_secret(get_details().length_shared_secret, 0);
        ::OQS_STATUS rv_ = ::OQS_KEM_encaps(
            kem_, ciphertext.data(), shared_secret.data(), public_key.data());
        if (rv_ != ::OQS_SUCCESS)
            throw std::runtime_error("Can not encapsulate secret");

        return std::make_pair(ciphertext, shared_secret);
    }

    bytes decap_secret(const bytes& ciphertext) const {
        bytes shared_secret(get_details().length_shared_secret, 0);
        ::OQS_STATUS rv_ = ::OQS_KEM_decaps(
            kem_, shared_secret.data(), ciphertext.data(), secret_key_.data());

        if (rv_ != ::OQS_SUCCESS)
            throw std::runtime_error("Can not decapsulate secret");

        return shared_secret;
    }

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

    friend std::ostream& operator<<(std::ostream& os,
                                    const KeyEncapsulation& key_encapsulation) {
        return os << "Key encapsulation mechanism: "
                  << key_encapsulation.get_details().name;
    }
}; // KeyEncapsulation

class Sigs final : public impl_details_::Singleton<const Sigs> {
    static std::size_t max_number_Sigs_;
    static std::vector<std::string> supported_Sigs_;
    static std::vector<std::string> enabled_Sigs_;

  public:
    Sigs() {
        for (std::size_t i = 0; i < max_number_Sigs_; ++i) {
            std::string alg_name = ::OQS_SIG_alg_identifier(i);
            supported_Sigs_.emplace_back(alg_name);
            if (is_Sig_enabled(get_Sig_name(i)))
                enabled_Sigs_.emplace_back(alg_name);
        }
    }

    static std::string get_Sig_name(std::size_t alg_id) {
        if (alg_id >= max_number_Sigs_)
            throw std::out_of_range("Algorithm ID out of range");

        return ::OQS_SIG_alg_identifier(alg_id);
    }

    static bool is_Sig_enabled(const std::string& alg_name) {
        ::OQS_SIG* sig = ::OQS_SIG_new(alg_name.c_str());
        if (sig) {
            OQS_SIG_free(sig);
            return true;
        }

        return false;
    }

    static bool is_Sig_supported(const std::string& alg_name) {
        return std::find(supported_Sigs_.begin(), supported_Sigs_.end(),
                         alg_name) != supported_Sigs_.end();
    }

    static const std::vector<std::string>& get_enabled_Sigs() {
        return enabled_Sigs_;
    }

    static const std::vector<std::string>& get_supported_Sigs() {
        return supported_Sigs_;
    }
};

std::size_t Sigs::max_number_Sigs_ = ::OQS_SIG_alg_count();
std::vector<std::string> Sigs::supported_Sigs_;
std::vector<std::string> Sigs::enabled_Sigs_;

class Signature {
    const std::string alg_name_;
    ::OQS_SIG* sig_;
    bytes secret_key_;

    struct alg_details_ {
        std::string name;
        std::string version;
        std::size_t claimed_nist_level;
        bool is_euf_cma;
        std::size_t length_public_key;
        std::size_t length_secret_key;
        std::size_t length_signature;
    } details_;

  public:
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

        sig_ = ::OQS_SIG_new(alg_name.c_str());

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

    virtual ~Signature() {
        if (!secret_key_.empty())
            ::OQS_MEM_cleanse(secret_key_.data(), secret_key_.size());
        ::OQS_SIG_free(sig_);
    }

    const alg_details_& get_details() const { return details_; }

    bytes generate_keypair() {
        bytes public_key(get_details().length_public_key, 0);
        secret_key_ = bytes(get_details().length_secret_key, 0);

        ::OQS_STATUS rv_ =
            ::OQS_SIG_keypair(sig_, public_key.data(), secret_key_.data());
        if (rv_ != ::OQS_SUCCESS)
            throw std::runtime_error("Can not generate keypair");

        return public_key;
    }

    bytes export_secret_key() const { return secret_key_; }

    bytes sign(const bytes& message) {
        bytes signature(get_details().length_signature, 0);
        std::size_t sig_len = 0;
        ::OQS_STATUS rv_ =
            ::OQS_SIG_sign(sig_, signature.data(), &sig_len, message.data(),
                           message.size(), secret_key_.data());

        if (rv_ != ::OQS_SUCCESS)
            throw std::runtime_error("Can not sign message");

        signature.resize(sig_len);

        return signature;
    }

    bool verify(const bytes& message, const bytes& signature,
                const bytes& public_key) {
        ::OQS_STATUS rv_ = ::OQS_SIG_verify(
            sig_, message.data(), message.size(), signature.data(),
            signature.size(), public_key.data());

        if (rv_ != ::OQS_SUCCESS)
            throw std::runtime_error("Can not verify signature");

        return true;
    }

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

    friend std::ostream& operator<<(std::ostream& os,
                                    const Signature& signature) {
        return os << "Signature mechanism: " << signature.get_details().name;
    }

}; // Signature

namespace impl_details_ {
// initialize the KEMs and Sigs singletons
static const KEMs& algs_ = KEMs::get_instance();
static const Sigs& sigs_ = Sigs::get_instance();
} // namespace impl_details_

} // namespace oqs

// dump hex strings
std::ostream& operator<<(std::ostream& os, const oqs::bytes& buf) {
    bool first = true;
    for (auto&& elem : buf) {
        if (first) {
            first = false;
            os << std::hex << std::uppercase << static_cast<int>(elem);
        } else {
            os << " " << std::hex << std::uppercase << static_cast<int>(elem);
        }
    }

    return os;
}

// dump vectors of strings
std::ostream& operator<<(std::ostream& os, const std::vector<std::string>& v) {
    bool first = true;
    for (auto&& elem : v) {
        if (first) {
            first = false;
            os << elem;
        } else {
            os << " " << elem;
        }
    }

    return os;
}

// convert C-strings to bytes (null terminator is not included)
oqs::bytes operator""_bytes(const char* c, std::size_t length) {
    oqs::bytes result(length);
    for (std::size_t i = 0; i < length; ++i)
        result[i] = static_cast<uint8_t>(c[i]);

    return result;
}

#endif // OQS_CPP_H_
