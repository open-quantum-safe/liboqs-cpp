// Unit testing oqs::Signature

#include <iostream>

#include <gtest/gtest.h>

#include "oqs_cpp.h"

TEST(oqs_Signature, AllTests) {
    std::cout << "Testing signatures:\n";
    for (auto&& sig : oqs::Sigs::get_enabled_Sigs()) {
        std::cout << sig << std::endl;

        oqs::bytes message = "This is our favourite message to sign"_bytes;

        oqs::Signature signer{sig};
        oqs::bytes signer_public_key = signer.generate_keypair();
        oqs::bytes signature = signer.sign(message);

        oqs::Signature verifier{sig};
        bool is_valid = verifier.verify(message, signature, signer_public_key);

        EXPECT_TRUE(is_valid);
    }
}