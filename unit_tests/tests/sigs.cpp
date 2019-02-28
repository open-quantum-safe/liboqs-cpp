// Unit testing oqs::Signature

#include <iostream>

#include <gtest/gtest.h>

#include "oqs_cpp.h"

TEST(oqs_Signature, Enabled) {
    std::cout << "Testing signatures:\n";
    for (auto&& sig : oqs::Sigs::get_enabled_sigs()) {
        std::cout << sig << '\n';

        oqs::bytes message = "This is our favourite message to sign"_bytes;

        oqs::Signature signer{sig};
        oqs::bytes signer_public_key = signer.generate_keypair();
        oqs::bytes signature = signer.sign(message);

        oqs::Signature verifier{sig};
        bool is_valid = verifier.verify(message, signature, signer_public_key);

        EXPECT_TRUE(is_valid);
    }
}

TEST(oqs_Signature, NotSupported) {
    EXPECT_THROW(oqs::Signature{"unsupported_sig"},
                 oqs::MechanismNotSupportedError);
}
