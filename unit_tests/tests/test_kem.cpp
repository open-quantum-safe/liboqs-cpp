// Unit testing oqs::KeyEncapsulation

#include <iostream>
#include <tuple>

#include <gtest/gtest.h>
#include <gmock/gmock.h>

#include "oqs_cpp.h"

bool test_kem(const std::string& kem_name) {
    oqs::KeyEncapsulation client{kem_name};
    oqs::bytes client_public_key = client.generate_keypair();
    oqs::KeyEncapsulation server{kem_name};
    oqs::bytes ciphertext, shared_secret_server;
    std::tie(ciphertext, shared_secret_server) =
        server.encap_secret(client_public_key);
    oqs::bytes shared_secret_client = client.decap_secret(ciphertext);
    for (std::size_t i = 0; i < shared_secret_client.size(); ++i)
        if (shared_secret_client[i] != shared_secret_server[i])
            return false;
    return true;
}

TEST(oqs_KeyEncapsulation, Enabled) {
    std::cout << "Testing enabled KEMs:\n";
    for (auto&& kem_name : oqs::KEMs::get_enabled_KEMs()) {
        std::cout << kem_name << '\n';
        EXPECT_TRUE(test_kem(kem_name));
    }
}

TEST(oqs_KeyEncapsulation, NotSupported) {
    EXPECT_THROW(oqs::KeyEncapsulation{"unsupported_kem"},
                 oqs::MechanismNotSupportedError);
}
