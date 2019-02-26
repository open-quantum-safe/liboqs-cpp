// Unit testing oqs::KeyEncapsulation

#include <tuple>

#include <gtest/gtest.h>
#include <gmock/gmock.h>

#include "oqs_cpp.h"

TEST(oqs_KeyEncapsulationMechanism, AllTests) {
    std::cout << "Testing KEMs:\n";
    for (auto&& kem : oqs::KEMs::get_enabled_KEMs()) {
        std::cout << kem << std::endl;

        oqs::KeyEncapsulation client{kem};
        oqs::bytes client_public_key = client.generate_keypair();

        oqs::KeyEncapsulation server{kem};
        oqs::bytes ciphertext, shared_secret_server;
        std::tie(ciphertext, shared_secret_server) =
            server.encap_secret(client_public_key);

        oqs::bytes shared_secret_client = client.decap_secret(ciphertext);

        EXPECT_THAT(shared_secret_client,
                    ::testing::ContainerEq(shared_secret_server));
    }
}