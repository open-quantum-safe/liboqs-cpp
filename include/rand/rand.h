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
inline bytes randombytes(std::size_t bytes_to_read) {
    bytes result(bytes_to_read);
    C::OQS_randombytes(result.data(), bytes_to_read);
    return result;
}

/**
 * \brief Generates \a bytes_to_read random bytes in-place
 * \note This implementation uses either the default RNG algorithm ("system"),
 * or whichever algorithm has been selected by
 * oqs::rand::randombytes_switch_algorithm()
 * \note \a bytes_to_read must not exceed the size of \a random_array
 * \param [in] bytes_to_read The number of random bytes to generate
 * \param [out] random_array Output vector of random bytes
 */
inline void randombytes(bytes& random_array, std::size_t bytes_to_read) {
    if (bytes_to_read > random_array.size())
        throw(std::out_of_range(
            "bytes_to_read exceeds the size of random_array"));
    C::OQS_randombytes(random_array.data(), bytes_to_read);
}

/**
 * \brief Switches the core OQS_randombytes to use the specified algorithm
 * \see <oqs/rand.h> liboqs header for more details.
 * \param alg_name Algorithm name, possible values are "system", "NIST-KAT", and
 * "OpenSSL", or the corresponding macros OQS_RAND_alg_system,
 * OQS_RAND_alg_nist_kat, and OQS_RAND_alg_openssl, respectively.
 */
inline void randombytes_switch_algorithm(const std::string& alg_name) {
    if (C::OQS_randombytes_switch_algorithm(alg_name.c_str()) != C::OQS_SUCCESS)
        throw std::runtime_error("Can not switch algorithm");
}

/**
 * \brief Initializes the NIST DRBG with the \a entropy_input seed. The security
 * parameter is 256 bits.
 * \param entropy_input Entropy input seed, must be exactly 48 bytes long
 * \param personalization_string Optional personalization string, which, if
 * non-empty, must be at least 48 bytes long
 */
inline void
randombytes_nist_kat_init_256bit(const bytes& entropy_input,
                                 const bytes& personalization_string = {}) {
    std::size_t len_str = personalization_string.size();

    if (entropy_input.size() != 48)
        throw std::out_of_range(
            "The entropy source must be exactly 48 bytes long");
    if (len_str > 0) {
        if (len_str < 48)
            throw std::out_of_range("The personalization string must be either "
                                    "empty or at least 48 bytes long");
        C::OQS_randombytes_nist_kat_init_256bit(entropy_input.data(),
                                                personalization_string.data());
        return;
    }
    C::OQS_randombytes_nist_kat_init_256bit(entropy_input.data(), nullptr);
}

/**
 * \brief Switches oqs::rand::randombytes() to use the given function
 * \note This allows additional custom RNGs besides the provided ones.
 * \param algorithm_ptr Pointer to RNG function
 */
inline void randombytes_custom_algorithm(void (*algorithm_ptr)(uint8_t*,
                                                               std::size_t)) {
    C::OQS_randombytes_custom_algorithm(algorithm_ptr);
}
} // namespace rand
} // namespace oqs

#endif // RAND_RAND_H_
