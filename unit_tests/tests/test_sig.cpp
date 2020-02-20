// Unit testing oqs::Signature

#include <iostream>
#include <thread>
#include <vector>

#include <gtest/gtest.h>

#include "oqs_cpp.h"

void test_sig(const std::string& sig_name, const oqs::bytes& msg) {
    oqs::Signature signer{sig_name};
    oqs::bytes signer_public_key = signer.generate_keypair();
    oqs::bytes signature = signer.sign(msg);
    oqs::Signature verifier{sig_name};
    bool is_valid = verifier.verify(msg, signature, signer_public_key);
    if (!is_valid)
        std::cerr << sig_name << ": signature verification failed" << std::endl;
    EXPECT_TRUE(is_valid);
}

TEST(oqs_Signature, Enabled) {
    oqs::bytes message = "This is our favourite message to sign"_bytes;
    std::vector<std::thread> thread_pool;
    thread_pool.reserve(oqs::Sigs::get_enabled_sigs().size());
    for (auto&& sig_name : oqs::Sigs::get_enabled_sigs()) {
        std::cout << sig_name << std::endl;
        thread_pool.emplace_back(std::thread(test_sig, sig_name, message));
    }
    for (auto&& elem : thread_pool)
        elem.join();
}

TEST(oqs_Signature, NotSupported) {
    EXPECT_THROW(oqs::Signature{"unsupported_sig"},
                 oqs::MechanismNotSupportedError);
}
