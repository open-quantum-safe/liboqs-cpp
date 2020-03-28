// Unit testing oqs::KeyEncapsulation

#include <iostream>
#include <mutex>
#include <thread>
#include <tuple>
#include <vector>

#include <gtest/gtest.h>

#include "oqs_cpp.h"

// no_thread_KEM_patterns lists KEM patterns that have issues running in a
// separate thread
static std::vector<std::string> no_thread_KEM_patterns{"Classic-McEliece",
                                                       "LEDAcryptKEM-LT52"};

// used for thread-safe console output
static std::mutex mu;

void test_kem(const std::string& kem_name) {
    {
        std::lock_guard<std::mutex> lg{mu};
        std::cout << kem_name << std::endl;
    }
    oqs::KeyEncapsulation client{kem_name};
    oqs::bytes client_public_key = client.generate_keypair();
    oqs::KeyEncapsulation server{kem_name};
    oqs::bytes ciphertext, shared_secret_server;
    std::tie(ciphertext, shared_secret_server) =
        server.encap_secret(client_public_key);
    oqs::bytes shared_secret_client = client.decap_secret(ciphertext);
    bool is_valid = (shared_secret_client == shared_secret_server);
    if (!is_valid)
        std::cerr << kem_name << ": shared secrets do not coincide"
                  << std::endl;
    EXPECT_TRUE(is_valid);
}

TEST(oqs_KeyEncapsulation, Enabled) {
    std::vector<std::thread> thread_pool;
    std::vector<std::string> enabled_KEMs = oqs::KEMs::get_enabled_KEMs();
    // first test KEMs that belong to no_thread_KEM_patterns[] in the main
    // thread (stack size is 8Mb on macOS), due to issues with stack size being
    // too small in macOS (512Kb for threads)
    for (auto&& kem_name : enabled_KEMs) {
        for (auto&& no_thread_kem : no_thread_KEM_patterns) {
            if (kem_name.find(no_thread_kem) != std::string::npos) {
                test_kem(kem_name);
            }
        }
    }
    // test the remaining KEMs in separate threads
    for (auto&& kem_name : enabled_KEMs) {
        bool test_in_thread = true;
        for (auto&& no_thread_kem : no_thread_KEM_patterns) {
            if (kem_name.find(no_thread_kem) != std::string::npos) {
                test_in_thread = false;
                break;
            }
        }
        if (test_in_thread)
            thread_pool.emplace_back(test_kem, kem_name);
    }
    // join the rest of the threads
    for (auto&& elem : thread_pool)
        elem.join();
}

TEST(oqs_KeyEncapsulation, NotSupported) {
    EXPECT_THROW(oqs::KeyEncapsulation{"unsupported_kem"},
                 oqs::MechanismNotSupportedError);
}
