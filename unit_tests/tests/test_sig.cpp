// Unit testing oqs::Signature

#include <iostream>
#include <mutex>
#include <thread>
#include <vector>

#include <gtest/gtest.h>

#include "oqs_cpp.h"

// no_thread_sig_patterns lists sig patterns that have issues running in a
// separate thread
static std::vector<std::string> no_thread_sig_patterns{"Rainbow-IIIc",
                                                       "Rainbow-Vc"};

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
        // use threads only for sigs that are not in no_thread_sig_patterns, due
        // to issues with stack size being too small in macOS (512Kb for
        // threads)
        bool test_in_thread = true;
        for (auto&& no_thread_sig : no_thread_sig_patterns) {
            if (sig_name.find(no_thread_sig) != std::string::npos) {
                test_in_thread = false;
                break;
            }
        }
        if (test_in_thread)
            thread_pool.emplace_back(test_sig, sig_name, message);
    }
    // test the other sigs in the main thread (stack size is 8Mb on macOS)
    for (auto&& sig_name : enabled_sigs) {
        for (auto&& no_thread_sig : no_thread_sig_patterns) {
            if (sig_name.find(no_thread_sig) != std::string::npos) {
                test_sig(sig_name, message);
            }
        }
    }
    // join the rest of the threads
    for (auto&& elem : thread_pool)
        elem.join();
}

TEST(oqs_Signature, NotSupported) {
    EXPECT_THROW(oqs::Signature{"unsupported_sig"},
                 oqs::MechanismNotSupportedError);
}
