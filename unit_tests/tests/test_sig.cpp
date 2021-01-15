// Unit testing oqs::Signature

#include <iostream>
#include <mutex>
#include <thread>
#include <vector>

#include <gtest/gtest.h>

#include "oqs_cpp.h"
#include "rand/rand.h"

// no_thread_sig_patterns lists sig patterns that have issues running in a
// separate thread
static std::vector<std::string> no_thread_sig_patterns{"Rainbow-III",
                                                       "Rainbow-V"};

// used for thread-safe console output
static std::mutex mu;

void test_sig_correctness(const std::string& sig_name, const oqs::bytes& msg) {
    {
        std::lock_guard<std::mutex> lg{mu};
        std::cout << "Correctness - " << sig_name << std::endl;
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

void test_sig_wrong_signature(const std::string& sig_name,
                              const oqs::bytes& msg) {
    {
        std::lock_guard<std::mutex> lg{mu};
        std::cout << "Wrong signature - " << sig_name << std::endl;
    }
    oqs::Signature signer{sig_name};
    oqs::bytes signer_public_key = signer.generate_keypair();
    oqs::bytes signature = signer.sign(msg);
    oqs::bytes wrong_signature = oqs::rand::randombytes(signature.size());
    oqs::Signature verifier{sig_name};
    bool is_valid = verifier.verify(msg, wrong_signature, signer_public_key);
    if (is_valid)
        std::cerr << sig_name << ": signature verification should have failed"
                  << std::endl;
    EXPECT_FALSE(is_valid);
}

void test_sig_wrong_public_key(const std::string& sig_name,
                               const oqs::bytes& msg) {
    {
        std::lock_guard<std::mutex> lg{mu};
        std::cout << "Wrong public key - " << sig_name << std::endl;
    }
    oqs::Signature signer{sig_name};
    oqs::bytes signer_public_key = signer.generate_keypair();
    oqs::bytes wrong_public_key =
        oqs::rand::randombytes(signer_public_key.size());
    oqs::bytes signature = signer.sign(msg);
    oqs::Signature verifier{sig_name};
    bool is_valid = verifier.verify(msg, signature, wrong_public_key);
    if (is_valid)
        std::cerr << sig_name << ": signature verification should have failed"
                  << std::endl;
    EXPECT_FALSE(is_valid);
}

TEST(oqs_Signature, Correctness) {
    oqs::bytes message = "This is our favourite message to sign"_bytes;
    std::vector<std::thread> thread_pool;
    std::vector<std::string> enabled_sigs = oqs::Sigs::get_enabled_sigs();
    // first test sigs that belong to no_thread_sig_patterns[] in the main
    // thread (stack size is 8Mb on macOS), due to issues with stack size being
    // too small in macOS (512Kb for threads)
    for (auto&& sig_name : enabled_sigs) {
        for (auto&& no_thread_sig : no_thread_sig_patterns) {
            if (sig_name.find(no_thread_sig) != std::string::npos) {
                test_sig_correctness(sig_name, message);
            }
        }
    }
    // test the remaining sigs in separate threads
    for (auto&& sig_name : enabled_sigs) {
        bool test_in_thread = true;
        for (auto&& no_thread_sig : no_thread_sig_patterns) {
            if (sig_name.find(no_thread_sig) != std::string::npos) {
                test_in_thread = false;
                break;
            }
        }
        if (test_in_thread)
            thread_pool.emplace_back(test_sig_correctness, sig_name, message);
    }
    // join the rest of the threads
    for (auto&& elem : thread_pool)
        elem.join();
}

TEST(oqs_Signature, WrongSignature) {
    oqs::bytes message = "This is our favourite message to sign"_bytes;
    std::vector<std::thread> thread_pool;
    std::vector<std::string> enabled_sigs = oqs::Sigs::get_enabled_sigs();
    // first test sigs that belong to no_thread_sig_patterns[] in the main
    // thread (stack size is 8Mb on macOS), due to issues with stack size being
    // too small in macOS (512Kb for threads)
    for (auto&& sig_name : enabled_sigs) {
        for (auto&& no_thread_sig : no_thread_sig_patterns) {
            if (sig_name.find(no_thread_sig) != std::string::npos) {
                test_sig_wrong_signature(sig_name, message);
            }
        }
    }
    // test the remaining sigs in separate threads
    for (auto&& sig_name : enabled_sigs) {
        bool test_in_thread = true;
        for (auto&& no_thread_sig : no_thread_sig_patterns) {
            if (sig_name.find(no_thread_sig) != std::string::npos) {
                test_in_thread = false;
                break;
            }
        }
        if (test_in_thread)
            thread_pool.emplace_back(test_sig_wrong_signature, sig_name,
                                     message);
    }
    // join the rest of the threads
    for (auto&& elem : thread_pool)
        elem.join();
}

TEST(oqs_Signature, WrongPublicKey) {
    oqs::bytes message = "This is our favourite message to sign"_bytes;
    std::vector<std::thread> thread_pool;
    std::vector<std::string> enabled_sigs = oqs::Sigs::get_enabled_sigs();
    // first test sigs that belong to no_thread_sig_patterns[] in the main
    // thread (stack size is 8Mb on macOS), due to issues with stack size being
    // too small in macOS (512Kb for threads)
    for (auto&& sig_name : enabled_sigs) {
        for (auto&& no_thread_sig : no_thread_sig_patterns) {
            if (sig_name.find(no_thread_sig) != std::string::npos) {
                test_sig_wrong_public_key(sig_name, message);
            }
        }
    }
    // test the remaining sigs in separate threads
    for (auto&& sig_name : enabled_sigs) {
        bool test_in_thread = true;
        for (auto&& no_thread_sig : no_thread_sig_patterns) {
            if (sig_name.find(no_thread_sig) != std::string::npos) {
                test_in_thread = false;
                break;
            }
        }
        if (test_in_thread)
            thread_pool.emplace_back(test_sig_wrong_public_key, sig_name,
                                     message);
    }
    // join the rest of the threads
    for (auto&& elem : thread_pool)
        elem.join();
}

TEST(oqs_Signature, NotSupported) {
    EXPECT_THROW(oqs::Signature{"unsupported_sig"},
                 oqs::MechanismNotSupportedError);
}
