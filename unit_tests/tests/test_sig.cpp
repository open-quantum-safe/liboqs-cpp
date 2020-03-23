// Unit testing oqs::Signature

#include <iostream>
#include <mutex>
#include <thread>
#include <vector>

#include <gtest/gtest.h>

#include "oqs_cpp.h"

// no_thread_sigs lists sigs that have issues running in a separate thread
static std::vector<std::string> no_thread_sigs{"Rainbow-IIIc-Classic",
                                               "Rainbow-IIIc-Cyclic",
                                               "Rainbow-IIIc-Cyclic-Compressed",
                                               "Rainbow-Vc-Classic",
                                               "Rainbow-Vc-Cyclic",
                                               "Rainbow-Vc-Cyclic-Compressed"};

// used for thread-safe console output
static std::mutex mu;

void test_sig(const std::string& sig_name, const oqs::bytes& msg) {
    {
        std::lock_guard<std::mutex> lg{mu};
        std::cout << sig_name << std::endl;
    }
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
    std::vector<std::string> enabled_sigs = oqs::Sigs::get_enabled_sigs();
    for (auto&& sig_name : enabled_sigs) {
        // use threads only for sigs that are not in no_thread_sigs, due to
        // issues with stack size being too small in macOS (512Kb for threads)
        if (std::find(std::begin(no_thread_sigs), std::end(no_thread_sigs),
                      sig_name) == std::end(no_thread_sigs))
            thread_pool.emplace_back(test_sig, sig_name, message);
    }
    // test the other sigs in the main thread (stack size is 8Mb on macOS)
    for (auto&& sig_name : no_thread_sigs)
        if (std::find(std::begin(enabled_sigs), std::end(enabled_sigs),
                      sig_name) != std::end(enabled_sigs))
            test_sig(sig_name, message);
    // join the rest of the threads
    for (auto&& elem : thread_pool)
        elem.join();
}

TEST(oqs_Signature, NotSupported) {
    EXPECT_THROW(oqs::Signature{"unsupported_sig"},
                 oqs::MechanismNotSupportedError);
}
