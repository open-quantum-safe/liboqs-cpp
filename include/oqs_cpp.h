/**
 * \file oqs_cpp.h
 * \brief Main header file for the liboqs C++ wrapper
 */

#ifndef OQS_CPP_H_
#define OQS_CPP_H_

#include <algorithm>
#include <chrono>
#include <cstdint>
#include <cstdlib>
#include <cstring>
#include <exception>
#include <iomanip>
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

/**
 * \namespace oqs::C
 * \brief Namespace containing all of the oqs C functions, so they do not
 * pollute the oqs namespace
 */
namespace C {
// everything in liboqs has C linkage
extern "C" {
#include <oqs/oqs.h>
}
} // namespace C

using byte = std::uint8_t;        ///< byte (unsigned)
using bytes = std::vector<byte>;  ///< vector of bytes (unsigned)
using OQS_STATUS = C::OQS_STATUS; ///< bring OQS_STATUS into the oqs namespace

/**
 * \namespace internal
 * \brief Internal implementation details
 */
namespace internal {
/* code from
https://github.com/vsoftco/qpp/blob/master/include/internal/classes/singleton.h
 */
/**
 * \class oqs::internal::Singleton
 * \brief Singleton class using CRTP pattern
 * \note Code from
 * https://github.com/vsoftco/qpp/blob/master/include/internal/classes/singleton.h
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

/**
 * \class oqs::internal::HexChop
 * \brief std::ostream manipulator for long vectors of oqs::byte, use it to
 * display only a small number of elements from the beginning and end of the
 * vector
 */
class HexChop {
    bytes v_; ///< vector of byes
    std::size_t start_,
        end_; ///< number of hex bytes taken from the start and from the end
    /**
     * \brief std::ostream manipulator
     * \param os Output stream
     * \param start Number of hex characters displayed from the beginning of
     * the vector
     * \param end  Number of hex characters displayed from the end of the vector
     * \param is_short Vector is too short, display all hex characters
     */
    void manipulate_ostream_(std::ostream& os, std::size_t start,
                             std::size_t end, bool is_short) const {
        std::ios_base::fmtflags saved{os.flags()}; // save the ostream flags
        os << std::setfill('0') << std::hex << std::uppercase;

        bool first = true;
        for (std::size_t i = 0; i < start; ++i) {
            if (first) {
                first = false;
                os << std::setw(2) << static_cast<int>(v_[i]);
            } else {
                os << " " << std::setw(2) << static_cast<int>(v_[i]);
            }
        }

        if (!is_short)
            os << " ... ";

        first = true;
        std::size_t v_size = v_.size();
        for (std::size_t i = v_size - end; i < v_size; ++i) {
            if (first) {
                first = false;
                os << static_cast<int>(v_[i]);
            } else
                os << " " << std::setw(2) << static_cast<int>(v_[i]);
        }

        os.flags(saved); // restore the ostream flags
    }

  public:
    /**
     * \brief Constructs an instance of oqs::internal::HexChop
     * \param v Vector of bytes
     * \param start Number of hex characters displayed from the beginning of
     * the vector
     * \param end  Number of hex characters displayed from the end of the vector
     */
    explicit HexChop(const oqs::bytes& v, std::size_t start, std::size_t end)
        : v_{v}, start_{start}, end_{end} {}

    /**
     * \brief std::ostream extraction operator for oqs::internal::HexChop
     * \param os Output stream
     * \param rhs oqs::internal::HexChop instance
     * \return Reference to the output stream
     */
    friend std::ostream& operator<<(std::ostream& os, const HexChop& rhs) {

        bool is_short = rhs.start_ + rhs.end_ >= rhs.v_.size();
        if (is_short)
            rhs.manipulate_ostream_(os, rhs.v_.size(), 0, true);
        else
            rhs.manipulate_ostream_(os, rhs.start_, rhs.end_, false);

        return os;
    }
}; // class HexChop
} // namespace internal

/* code from
https://github.com/vsoftco/qpp/blob/master/include/classes/timer.h
 */
/**
 * \class oqs::Timer
 * \brief High resolution timer
 * \note Code from
 * https://github.com/vsoftco/qpp/blob/master/include/classes/timer.h
 * \tparam T Tics duration, default is std::chrono::duration<double>,
 * i.e. seconds in double precision
 * \tparam CLOCK_T Clock's type, default is std::chrono::steady_clock,
 * not affected by wall clock changes during runtime
 */
template <typename T = std::chrono::duration<double>,
          typename CLOCK_T = std::chrono::steady_clock>
class Timer {
  protected:
    typename CLOCK_T::time_point start_, end_;

  public:
    /**
     * \brief Constructs an instance with the current time as the start point
     */
    Timer() noexcept : start_{CLOCK_T::now()}, end_{start_} {}

    /**
     * \brief Resets the chronometer
     *
     * Resets the start/end point to the current time
     */
    void tic() noexcept { start_ = end_ = CLOCK_T::now(); }

    /**
     * \brief Stops the chronometer
     *
     * Set the current time as the end point
     *
     * \return Reference to the current instance
     */
    const Timer& toc() & noexcept {
        end_ = CLOCK_T::now();
        return *this;
    }

    /**
     * \brief Time passed in the duration specified by T
     *
     * \return Number of tics (specified by T) that passed between the
     * instantiation/reset and invocation of oqs::Timer::toc()
     */
    double tics() const noexcept {
        return std::chrono::duration_cast<T>(end_ - start_).count();
    }

    /**
     * \brief Duration specified by U
     *
     * \tparam U Duration, default is T, which defaults to
     * std::chrono::duration<double>, i.e. seconds in double precision
     *
     * \return Duration that passed between the
     * instantiation/reset and invocation of oqs::Timer::toc()
     */
    template <typename U = T>
    U get_duration() const noexcept {
        return std::chrono::duration_cast<U>(end_ - start_);
    }

    /**
     * \brief Default virtual destructor
     */
    virtual ~Timer() = default;

    friend std::ostream& operator<<(std::ostream& os, const Timer& rhs) {
        return os << rhs.tics();
    }
}; // class Timer

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
     * \brief Maximum number of supported KEMs
     * \return Maximum number of supported KEMs
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
        C::OQS_KEM* kem = C::OQS_KEM_new(alg_name.c_str());
        if (kem) {
            C::OQS_KEM_free(kem);
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
    std::string alg_name_; ///< cryptographic algorithm name
    std::shared_ptr<C::OQS_KEM> kem_{nullptr, [](C::OQS_KEM* p) {
                                         C::OQS_KEM_free(p);
                                     }}; ///< liboqs smart pointer to C::OQS_KEM
    bytes secret_key_{};                 ///< secret key

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
    explicit KeyEncapsulation(const std::string& alg_name,
                              const bytes& secret_key = {})
        : alg_name_{alg_name}, secret_key_{secret_key} {
        // KEM not enabled
        if (!KEMs::is_KEM_enabled(alg_name)) {
            // perhaps it's supported
            if (KEMs::is_KEM_supported(alg_name))
                throw MechanismNotEnabledError(alg_name);
            else
                throw MechanismNotSupportedError(alg_name);
        }

        kem_.reset(C::OQS_KEM_new(alg_name.c_str()));

        details_.name = kem_->method_name;
        details_.version = kem_->alg_version;
        details_.claimed_nist_level = kem_->claimed_nist_level;
        details_.is_ind_cca = kem_->ind_cca;
        details_.length_public_key = kem_->length_public_key;
        details_.length_secret_key = kem_->length_secret_key;
        details_.length_ciphertext = kem_->length_ciphertext;
        details_.length_shared_secret = kem_->length_shared_secret;
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
    KeyEncapsulation(KeyEncapsulation&& rhs)
        : alg_name_{std::move(rhs.alg_name_)}, kem_{std::move(rhs.kem_)},
          secret_key_{std::move(rhs.secret_key_)}, details_{std::move(
                                                       rhs.details_)} {
        rhs.secret_key_.resize(details_.length_secret_key);
        C::OQS_MEM_cleanse(rhs.secret_key_.data(), rhs.secret_key_.size());
        rhs.secret_key_.resize(0);
    }
    /**
     * \brief Move assignment operator, guarantees that the rvalue secret key is
     * always zeroed
     * \param rhs oqs::KeyEncapsulation instance
     * \return Reference to the current instance
     */
    KeyEncapsulation& operator=(KeyEncapsulation&& rhs) {
        alg_name_ = std::move(rhs.alg_name_);
        kem_ = std::move(rhs.kem_);
        secret_key_ = std::move(rhs.secret_key_);
        details_ = std::move(rhs.details_);
        rhs.secret_key_.resize(details_.length_secret_key);
        C::OQS_MEM_cleanse(rhs.secret_key_.data(), rhs.secret_key_.size());
        rhs.secret_key_.resize(0);

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
    const alg_details_& get_details() const& { return details_; }

    /**
     * \brief KEM algorithm details, rvalue overload
     * \return KEM algorithm details
     */
    alg_details_ get_details() const&& { return details_; }

    /**
     * \brief Generate public key/secret key pair
     * \return Public key
     */
    bytes generate_keypair() {
        bytes public_key(details_.length_public_key, 0);
        secret_key_ = bytes(details_.length_secret_key, 0);

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
        if (public_key.size() != details_.length_public_key)
            throw std::runtime_error("Incorrect public key length");

        bytes ciphertext(details_.length_ciphertext, 0);
        bytes shared_secret(details_.length_shared_secret, 0);
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
        if (ciphertext.size() != details_.length_ciphertext)
            throw std::runtime_error("Incorrect ciphertext length");

        if (secret_key_.size() != details_.length_secret_key)
            throw std::runtime_error(
                "Incorrect secret key length, make sure you "
                "specify one in the constructor or run "
                "oqs::Signature::generate_keypair()");

        bytes shared_secret(details_.length_shared_secret, 0);
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
     * \param rhs oqs::KeyEncapsulation instance
     * \return Reference to the output stream
     */
    friend std::ostream& operator<<(std::ostream& os,
                                    const KeyEncapsulation& rhs) {
        return os << "Key encapsulation mechanism: " << rhs.details_.name;
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
     * \brief Maximum number of supported signatures
     * \return Maximum number of supported signatures
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
        C::OQS_SIG* sig = C::OQS_SIG_new(alg_name.c_str());
        if (sig) {
            C::OQS_SIG_free(sig);
            return true;
        }

        return false;
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
    std::string alg_name_; ///< cryptographic algorithm name
    std::shared_ptr<C::OQS_SIG> sig_{nullptr, [](C::OQS_SIG* p) {
                                         C::OQS_SIG_free(p);
                                     }}; ///< liboqs smart pointer to C::OQS_SIG
    bytes secret_key_{};                 ///< secret key

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
    explicit Signature(const std::string& alg_name,
                       const bytes& secret_key = {})
        : alg_name_{alg_name}, secret_key_{secret_key} {
        // signature not enabled
        if (!Sigs::is_sig_enabled(alg_name)) {
            // perhaps it's supported
            if (Sigs::is_sig_supported(alg_name))
                throw MechanismNotEnabledError(alg_name);
            else
                throw MechanismNotSupportedError(alg_name);
        }

        sig_.reset(C::OQS_SIG_new(alg_name.c_str()));

        details_.name = sig_->method_name;
        details_.version = sig_->alg_version;
        details_.claimed_nist_level = sig_->claimed_nist_level;
        details_.is_euf_cma = sig_->euf_cma;
        details_.length_public_key = sig_->length_public_key;
        details_.length_secret_key = sig_->length_secret_key;
        details_.length_signature = sig_->length_signature;
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
    Signature(Signature&& rhs)
        : alg_name_{std::move(rhs.alg_name_)}, sig_{std::move(rhs.sig_)},
          secret_key_{std::move(rhs.secret_key_)}, details_{std::move(
                                                       rhs.details_)} {
        rhs.secret_key_.resize(details_.length_secret_key);
        C::OQS_MEM_cleanse(rhs.secret_key_.data(), rhs.secret_key_.size());
        rhs.secret_key_.resize(0);
    }
    /**
     * \brief Move assignment operator, guarantees that the rvalue secret key is
     * always zeroed
     * \param rhs oqs::Signature instance
     * \return Reference to the current instance
     */
    Signature& operator=(Signature&& rhs) {
        alg_name_ = std::move(rhs.alg_name_);
        sig_ = std::move(rhs.sig_);
        secret_key_ = std::move(rhs.secret_key_);
        details_ = std::move(rhs.details_);
        rhs.secret_key_.resize(details_.length_secret_key);
        C::OQS_MEM_cleanse(rhs.secret_key_.data(), rhs.secret_key_.size());
        rhs.secret_key_.resize(0);

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
    const alg_details_& get_details() const& { return details_; }

    /**
     * \brief Signature algorithm details, rvalue overload
     * \return Signature algorithm details
     */
    alg_details_ get_details() const&& { return details_; }

    /**
     * \brief Generate public key/secret key pair
     * \return Public key
     */
    bytes generate_keypair() {
        bytes public_key(get_details().length_public_key, 0);
        secret_key_ = bytes(details_.length_secret_key, 0);

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
        if (secret_key_.size() != details_.length_secret_key)
            throw std::runtime_error(
                "Incorrect secret key length, make sure you "
                "specify one in the constructor or run "
                "oqs::Signature::generate_keypair()");

        bytes signature(details_.length_signature, 0);
        std::size_t sig_len = 0;
        OQS_STATUS rv_ =
            C::OQS_SIG_sign(sig_.get(), signature.data(), &sig_len,
                            message.data(), message.size(), secret_key_.data());

        if (rv_ != OQS_STATUS::OQS_SUCCESS)
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
                const bytes& public_key) const {
        if (public_key.size() != details_.length_public_key)
            throw std::runtime_error("Incorrect public key length");

        OQS_STATUS rv_ = C::OQS_SIG_verify(sig_.get(), message.data(),
                                           message.size(), signature.data(),
                                           signature.size(), public_key.data());

        return rv_ == OQS_STATUS::OQS_SUCCESS ? true : false;
    }

    /**
     * \brief std::ostream extraction operator for the signature algorithm
     * details
     * \param os Output stream
     * \param rhs Algorithm details instance
     * \return Reference to the output stream
     */
    friend std::ostream& operator<<(std::ostream& os, const alg_details_& rhs) {
        os << "Name: " << rhs.name << '\n';
        os << "Version: " << rhs.version << '\n';
        os << "Claimed NIST level: " << rhs.claimed_nist_level << '\n';
        os << "Is EUF_CMA: " << rhs.is_euf_cma << '\n';
        os << "Length public key (bytes): " << rhs.length_public_key << '\n';
        os << "Length secret key (bytes): " << rhs.length_secret_key << '\n';
        os << "Maximum length signature (bytes): " << rhs.length_signature;
        return os;
    }

    /**
     * \brief std::ostream extraction operator for oqs::Signature
     * \param os Output stream
     * \param rhs oqs::Signature instance
     * \return Reference to the output stream
     */
    friend std::ostream& operator<<(std::ostream& os, const Signature& rhs) {
        return os << "Signature mechanism: " << rhs.details_.name;
    }
}; // class Signature

namespace internal {
// initialize the KEMs and Sigs singletons
static const KEMs& algs_ =
    KEMs::get_instance(); ///< initializes the KEMs singleton
static const Sigs& sigs_ =
    Sigs::get_instance(); ///< initializes the Sigs singleton
} // namespace internal

/**
 * \brief Constructs an instance of oqs::internal::HexChop
 * \param v Vector of bytes
 * \param start Number of hex characters displayed from the beginning of
 * the vector
 * \param end  Number of hex characters displayed from the end of the vector
 * \return Instance of oqs::internal::HexChop
 */
inline internal::HexChop hex_chop(const oqs::bytes& v, std::size_t start = 8,
                                  std::size_t end = 8) {
    return internal::HexChop(v, start, end);
}
} // namespace oqs

/**
 * \brief std::ostream extraction operator for oqs::bytes
 * \param os Output stream
 * \param rhs Vector of oqs::byte
 * \return Reference to the output stream
 */
inline std::ostream& operator<<(std::ostream& os, const oqs::bytes& rhs) {
    return os << oqs::hex_chop(rhs, rhs.size(), 0);
}

/**
 * \brief std::ostream extraction operator for vectors of strings
 * \param os Output stream
 * \param rhs Vector of std::string
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
