// various RNGs C++ example

#include <cstdint>
#include <cstdlib>
#include <iostream>

// RNG support
#include "rand/rand.hpp"

// CustomRNG provides a (trivial) custom random number generator; the memory is
// provided by the caller, i.e. oqs::rand::randombytes()
void custom_RNG(uint8_t* random_array, std::size_t bytes_to_read) {
    for (std::size_t i = 0; i < bytes_to_read; ++i)
        random_array[i] = static_cast<oqs::byte>(i % 256);
}

int main() {
    std::cout << "liboqs version: " << oqs::oqs_version() << '\n';
    std::cout << "liboqs-cpp version: " << oqs::oqs_cpp_version() << '\n';

    oqs::rand::randombytes_switch_algorithm(OQS_RAND_alg_system);
    std::cout << std::setw(18) << std::left;
    std::cout << "System (default): " << oqs::rand::randombytes(32) << '\n';

    oqs::rand::randombytes_custom_algorithm(custom_RNG);
    std::cout << std::setw(18) << std::left;
    std::cout << "Custom RNG: " << oqs::rand::randombytes(32) << '\n';

// We do not yet support OpenSSL on Windows
#ifndef _WIN32
    oqs::rand::randombytes_switch_algorithm(OQS_RAND_alg_openssl);
    std::cout << std::setw(18) << std::left;
    std::cout << "OpenSSL: " << oqs::rand::randombytes(32) << '\n';
#endif
}
