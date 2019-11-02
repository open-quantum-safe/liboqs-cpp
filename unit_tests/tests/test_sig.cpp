// Unit testing oqs::Signature

#include <iostream>

#include <gtest/gtest.h>

#include "oqs_cpp.h"

bool test_sig(const std::string& sig_name, const oqs::bytes& msg) {
    oqs::Signature signer{sig_name};
    oqs::bytes signer_public_key = signer.generate_keypair();
    oqs::bytes signature = signer.sign(msg);
    oqs::Signature verifier{sig_name};
    return verifier.verify(msg, signature, signer_public_key);
}

TEST(oqs_Signature, Enabled) {
    std::cout << "Testing enabled signatures:\n";
    oqs::bytes message = "This is our favourite message to sign"_bytes;
    for (auto&& sig_name : oqs::Sigs::get_enabled_sigs()) {
        std::cout << sig_name << '\n';
        EXPECT_TRUE(test_sig(sig_name, message));
    }
}

TEST(oqs_Signature, NotSupported) {
    EXPECT_THROW(oqs::Signature{"unsupported_sig"},
                 oqs::MechanismNotSupportedError);
}
