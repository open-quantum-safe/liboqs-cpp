#ifndef LIBOQS_CPP_H_
#define LIBOQS_CPP_H_

// everything in liboqs has C linkage
extern "C" {
#include <oqs/oqs.h>
}

#include <algorithm>
#include <cstdlib>
#include <exception>
#include <string>
#include <vector>

namespace oqs {

namespace details_ {
// code from
// https://github.com/vsoftco/qpp/blob/master/include/internal/classes/singleton.h
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
} // namespace details_

class MechanismNotSupportedError : public std::runtime_error {
  public:
    MechanismNotSupportedError(const std::string& alg_name)
        : std::runtime_error{alg_name + " is not supported by OQS"} {}
}; // class MechanismNotSupportedError

class MechanismNotEnabledError : public std::runtime_error {
  public:
    MechanismNotEnabledError(const std::string& alg_name)
        : std::runtime_error{alg_name + " is not supported by OQS"} {}
}; // class MechanismNotEnabledError

class KEMs : public details_::Singleton<const KEMs> {
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

    static std::vector<std::string> get_enabled_KEMs() { return enabled_KEMs_; }

    static std::vector<std::string> get_supported_KEMs() {
        return supported_KEMs_;
    }
}; // KEMs

std::size_t KEMs::max_number_KEMs_ = ::OQS_KEM_alg_count();
std::vector<std::string> KEMs::supported_KEMs_;
std::vector<std::string> KEMs::enabled_KEMs_;

namespace details_ {
// initialize the KEMs singleton
static const KEMs& algs_ = KEMs::get_instance();
} // namespace details_

class KeyEncapsulation {
    ::OQS_KEM oqs_kem_;
};

} // namespace oqs
#endif // LIBOQS_CPP_H_
