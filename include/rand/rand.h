/**
 * \file rand/rand.h
 * \brief Provides support for various RNG-related functions
 */

#ifndef RAND_RAND_H_
#define RAND_RAND_H_

#include <cstdint>
#include <cstdlib>
#include <cstring>
#include <stdexcept>
#include <string>
#include <vector>

#include "common.h"

namespace oqs {
namespace C {
// everything in liboqs has C linkage
extern "C" {
#include <oqs/rand.h>
}
} // namespace C

/**
 * \namespace rand
 * \brief Namespace containing RNG-related functions
 */
namespace rand {
/**
 * \brief Generates \a bytes_to_read random bytes
 * \note This implementation uses either the default RNG algorithm ("system"),
 * or whichever algorithm has been selected by
 * oqs::rand::randombytes_switch_algorithm()
 * \param bytes_to_read The number of random bytes to generate
 * \return Vector of random bytes
 */
bytes randombytes(std::size_t bytes_to_read) {
    bytes result(bytes_to_read);
    C::OQS_randombytes(result.data(), bytes_to_read);
    return result;
}

/**
 * \brief Switches the core OQS_randombytes to use the specified algorithm
 * \param alg_name Algorithm name, possible values are "system", "NIST-KAT",
 * "OpenSSL". See <oqs/rand.h> liboqs header for more details.
 */
void randombytes_switch_algorithm(const std::string& alg_name) {
    if (C::OQS_randombytes_switch_algorithm(alg_name.c_str()) != C::OQS_SUCCESS)
        throw std::runtime_error("Can not switch algorithm");
}

/**
 * \brief Initializes the NIST DRBG with the \a entropy_input seed
 * \param entropy_input Entropy input seed, must be exactly 48 bytes long
 * \param personalization_string Optional personalization string, which, if
 * non-empty, must be at least 48 bytes long
 */
void randombytes_nist_kat_init(const bytes& entropy_input,
                               const bytes& personalization_string = {}) {
    std::size_t len_str = personalization_string.size();

    if (entropy_input.size() != 48)
        throw std::out_of_range(
            "The entropy source must be exactly 48 bytes long");
    if (len_str > 0) {
        if (len_str < 48)
            throw std::out_of_range("The personalization string must be either "
                                    "empty or at least 48 bytes long");
        C::OQS_randombytes_nist_kat_init(entropy_input.data(),
                                         personalization_string.data(), 256);
        return;
    }
    C::OQS_randombytes_nist_kat_init(entropy_input.data(), nullptr, 256);
}

namespace internal {
using algorithm_ptr_fn = bytes (*)(size_t);

/**
 * \brief Global RNG algorithm callback
 * \return Global RNG algorithm callback set by
 * oqs::rand::randombytes_custom_algorithm()
 */
algorithm_ptr_fn& get_algorithm_ptr_fn() {
    static algorithm_ptr_fn algorithm_ptr_callback{nullptr};
    return algorithm_ptr_callback;
}

/**
 * \brief Automatically invoked by oqs::rand::randombytes_custom_algorithm()
 * \note When invoked, the memory is provided by the caller, i.e. by
 * oqs::rand::randombytes()
 * \param random_array Pointer to memory to be filled in
 * \param bytes_to_read The number of (random) bytes to generate
 */
void algorithm_ptr(uint8_t* random_array, size_t bytes_to_read) {
    algorithm_ptr_fn& algorithm_ptr_callback = get_algorithm_ptr_fn();
    if (algorithm_ptr_callback == nullptr)
        throw std::runtime_error("the RNG algorithm callback is not set");

    bytes result = algorithm_ptr_callback(bytes_to_read);
    std::memcpy(random_array, result.data(), bytes_to_read);
}
} // namespace internal

/**
 * \brief Switches oqs::rand::randombytes() to use the given function
 * \note This allows additional custom RNGs besides the provided ones. The
 * provided RNG function must have the same signature as
 * oqs::rand::randombytes(), i.e. bytes (*)(std::size_t).
 * \param algorithm_ptr Pointer to RNG function
 */
void randombytes_custom_algorithm(bytes (*algorithm_ptr)(std::size_t)) {
    internal::get_algorithm_ptr_fn() = algorithm_ptr;
    C::OQS_randombytes_custom_algorithm(internal::algorithm_ptr);
}
} // namespace rand
} // namespace oqs

#endif // RAND_RAND_H_
