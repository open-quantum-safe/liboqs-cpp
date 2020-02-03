// Unit testing oqs::KeyEncapsulation

#include <iostream>
#include <mutex>
#include <thread>
#include <tuple>
#include <vector>

#include <gtest/gtest.h>

#include "oqs_cpp.h"

//static std::mutex mut;

void test_kem(const std::string& kem_name) {
    oqs::KeyEncapsulation client{kem_name};
    oqs::bytes client_public_key = client.generate_keypair();
    oqs::KeyEncapsulation server{kem_name};
    oqs::bytes ciphertext, shared_secret_server;
    std::tie(ciphertext, shared_secret_server) =
        server.encap_secret(client_public_key);
    oqs::bytes shared_secret_client = client.decap_secret(ciphertext);
    bool is_valid = (shared_secret_client == shared_secret_server);
    {
        //std::lock_guard<std::mutex> lock{mut};
        EXPECT_TRUE(is_valid);
        if (!is_valid)
            std::cout << kem_name << ": shared secrets do not coincide"
                      << std::endl;
    }
}

TEST(oqs_KeyEncapsulation, Enabled) {
//    std::vector<std::thread> thread_pool;
//    thread_pool.reserve(oqs::KEMs::get_enabled_KEMs().size());
    for (auto&& kem_name : oqs::KEMs::get_enabled_KEMs()) {
        std::cout << kem_name << std::endl;
        test_kem(kem_name);
//        thread_pool.emplace_back(std::thread(test_kem, kem_name));
    }
//    for (auto&& elem : thread_pool)
//        elem.join();
}

TEST(oqs_KeyEncapsulation, NotSupported) {
    EXPECT_THROW(oqs::KeyEncapsulation{"unsupported_kem"},
                 oqs::MechanismNotSupportedError);
}
