// Unit testing oqs::Signature

#include <algorithm>
#include <exception>
#include <iostream>
#include <mutex>
#include <thread>
#include <utility>
#include <vector>

#include <gtest/gtest.h>

#include "oqs_cpp.hpp"
#include "rand/rand.hpp"

// Signatures that need the main thread's larger stack on some platforms.
static std::vector<std::string> no_thread_sig_patterns{
    "cross-rsdp", "cross-rsdpg", "SPHINCS+", "Falcon", "MAYO", "SNOVA"};
static std::mutex mu;

namespace {
// Exported key copies belong to the caller. Clean up these test-owned copies,
// including when an assertion returns early or an operation throws.
class TestSecret {
    oqs::bytes value_;

  public:
    explicit TestSecret(oqs::bytes value) noexcept {
        value_.swap(value);
    }
    TestSecret(const TestSecret&) = delete;
    TestSecret& operator=(const TestSecret&) = delete;
    ~TestSecret() {
        if (!value_.empty())
            oqs::C::OQS_MEM_cleanse(value_.data(), value_.size());
    }
    const oqs::bytes& get() const { return value_; }
};
} // namespace

// The -extmu variants accept exactly 64 bytes of externally computed mu.
oqs::bytes test_message(const std::string& sig_name) {
    if (sig_name.find("-extmu") != std::string::npos)
        return oqs::rand::randombytes(64);
    return "This is our favourite message to sign"_bytes;
}

void test_sig_correctness(const std::string& sig_name, const oqs::bytes& msg) {
    {
        std::lock_guard<std::mutex> lg{mu};
        std::cout << "Correctness - " << sig_name << std::endl;
    }
    oqs::Signature signer{sig_name};
    oqs::bytes signer_public_key = signer.generate_keypair();
    oqs::bytes signature = signer.sign(msg);
    oqs::Signature verifier{sig_name};
    EXPECT_TRUE(verifier.verify(msg, signature, signer_public_key));
}

void test_sig_correctness_with_ctx_str(const std::string& sig_name,
                                       const oqs::bytes& msg) {
    oqs::Signature signer{sig_name};
    if (!signer.get_details().sig_with_ctx_support)
        return;
    {
        std::lock_guard<std::mutex> lg{mu};
        std::cout << "Correctness with context string - " << sig_name
                  << std::endl;
    }
    oqs::bytes context_str{"some context"_bytes};
    oqs::bytes signer_public_key = signer.generate_keypair();
    oqs::bytes signature = signer.sign_with_ctx_str(msg, context_str);
    oqs::Signature verifier{sig_name};
    EXPECT_TRUE(verifier.verify_with_ctx_str(msg, signature, context_str,
                                            signer_public_key));
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
    EXPECT_FALSE(verifier.verify(msg, wrong_signature, signer_public_key));
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
    EXPECT_FALSE(verifier.verify(msg, signature, wrong_public_key));
}

void test_sig_in_thread(void (*test)(const std::string&, const oqs::bytes&),
                        const std::string& sig_name, const oqs::bytes& msg) {
    try {
        test(sig_name, msg);
    } catch (const std::exception& e) {
        ADD_FAILURE() << sig_name << ": unexpected exception: " << e.what();
    } catch (...) {
        ADD_FAILURE() << sig_name << ": unexpected exception";
    }
}

TEST(oqs_Signature, Correctness) {
    const auto& enabled = oqs::Sigs::get_enabled_sigs();
    if (enabled.empty())
        GTEST_SKIP() << "No enabled SIG algorithms";

    std::vector<std::thread> thread_pool;
    // Run algorithms requiring a larger stack on the main thread first.
    for (const auto& alg : enabled) {
        for (const auto& pattern : no_thread_sig_patterns) {
            if (alg.find(pattern) != std::string::npos) {
                test_sig_correctness(alg, test_message(alg));
                break;
            }
        }
    }
    for (const auto& alg : enabled) {
        bool test_in_thread = true;
        for (const auto& pattern : no_thread_sig_patterns) {
            if (alg.find(pattern) != std::string::npos) {
                test_in_thread = false;
                break;
            }
        }
        if (test_in_thread)
            thread_pool.emplace_back(test_sig_in_thread,
                                     test_sig_correctness, alg, test_message(alg));
    }
    for (auto& thread : thread_pool)
        thread.join();
}

TEST(oqs_Signature, CorrectnessWithContextString) {
    const auto& enabled = oqs::Sigs::get_enabled_sigs();
    if (enabled.empty())
        GTEST_SKIP() << "No enabled SIG algorithms";

    std::vector<std::thread> thread_pool;
    // Run algorithms requiring a larger stack on the main thread first.
    for (const auto& alg : enabled) {
        for (const auto& pattern : no_thread_sig_patterns) {
            if (alg.find(pattern) != std::string::npos) {
                test_sig_correctness_with_ctx_str(alg, test_message(alg));
                break;
            }
        }
    }
    for (const auto& alg : enabled) {
        bool test_in_thread = true;
        for (const auto& pattern : no_thread_sig_patterns) {
            if (alg.find(pattern) != std::string::npos) {
                test_in_thread = false;
                break;
            }
        }
        if (test_in_thread)
            thread_pool.emplace_back(test_sig_in_thread,
                                     test_sig_correctness_with_ctx_str, alg, test_message(alg));
    }
    for (auto& thread : thread_pool)
        thread.join();
}

TEST(oqs_Signature, WrongSignature) {
    const auto& enabled = oqs::Sigs::get_enabled_sigs();
    if (enabled.empty())
        GTEST_SKIP() << "No enabled SIG algorithms";

    std::vector<std::thread> thread_pool;
    // Run algorithms requiring a larger stack on the main thread first.
    for (const auto& alg : enabled) {
        for (const auto& pattern : no_thread_sig_patterns) {
            if (alg.find(pattern) != std::string::npos) {
                test_sig_wrong_signature(alg, test_message(alg));
                break;
            }
        }
    }
    for (const auto& alg : enabled) {
        bool test_in_thread = true;
        for (const auto& pattern : no_thread_sig_patterns) {
            if (alg.find(pattern) != std::string::npos) {
                test_in_thread = false;
                break;
            }
        }
        if (test_in_thread)
            thread_pool.emplace_back(test_sig_in_thread,
                                     test_sig_wrong_signature, alg, test_message(alg));
    }
    for (auto& thread : thread_pool)
        thread.join();
}

TEST(oqs_Signature, WrongPublicKey) {
    const auto& enabled = oqs::Sigs::get_enabled_sigs();
    if (enabled.empty())
        GTEST_SKIP() << "No enabled SIG algorithms";

    std::vector<std::thread> thread_pool;
    // Run algorithms requiring a larger stack on the main thread first.
    for (const auto& alg : enabled) {
        for (const auto& pattern : no_thread_sig_patterns) {
            if (alg.find(pattern) != std::string::npos) {
                test_sig_wrong_public_key(alg, test_message(alg));
                break;
            }
        }
    }
    for (const auto& alg : enabled) {
        bool test_in_thread = true;
        for (const auto& pattern : no_thread_sig_patterns) {
            if (alg.find(pattern) != std::string::npos) {
                test_in_thread = false;
                break;
            }
        }
        if (test_in_thread)
            thread_pool.emplace_back(test_sig_in_thread,
                                     test_sig_wrong_public_key, alg, test_message(alg));
    }
    for (auto& thread : thread_pool)
        thread.join();
}

namespace {
void expect_sig_key_works(const oqs::Signature& key,
                          const oqs::bytes& public_key) {
    const auto message = test_message(key.get_details().name);
    const auto signature = key.sign(message);
    oqs::Signature verifier{key.get_details().name};
    EXPECT_TRUE(verifier.verify(message, signature, public_key));
}
} // namespace

TEST(oqs_Signature, NotSupported) {
    EXPECT_THROW(oqs::Signature{"unsupported_sig"},
                 oqs::MechanismNotSupportedError);
}

TEST(oqs_Signature, NotSupportedWithSecretKey) {
    // This checks the exception and preservation of a caller-owned lvalue.
    // Cleansing on failure is tested separately by the instrumented harness.
    const TestSecret caller_key{oqs::bytes(32, 0xa5)};
    EXPECT_THROW((oqs::Signature{"unsupported_sig", caller_key.get()}),
                 oqs::MechanismNotSupportedError);
    EXPECT_TRUE(std::all_of(caller_key.get().begin(), caller_key.get().end(),
                            [](unsigned char byte) { return byte == 0xa5; }));
}

TEST(oqs_Signature, SecretKeyLifecycle) {
    const auto& enabled = oqs::Sigs::get_enabled_sigs();
    if (enabled.empty())
        GTEST_SKIP() << "No enabled SIG algorithms";

    // Prefer a fast standard algorithm for repeated lifecycle operations.
    // Run this test on the main thread, including the fallback algorithm.
    const auto preferred = std::find(enabled.begin(), enabled.end(),
                                     "ML-DSA-65");
    const auto& alg = preferred != enabled.end() ? *preferred : enabled.front();
    SCOPED_TRACE(alg);

    oqs::Signature source{alg};
    const auto public_key = source.generate_keypair();
    const TestSecret original_key{source.export_secret_key()};
    ASSERT_EQ(original_key.get().size(), source.get_details().length_secret_key);

    // Copy construction preserves the source and yields a usable key.
    oqs::Signature copied{source};
    EXPECT_EQ(TestSecret{copied.export_secret_key()}.get(), original_key.get());
    expect_sig_key_works(copied, public_key);

    // Exercise replacement across algorithms when the alternative is enabled.
    const auto alternative = std::find(enabled.begin(), enabled.end(),
                                       "ML-DSA-44");
    const auto& target_alg = alternative != enabled.end() ? *alternative : alg;
    oqs::Signature target{target_alg};
    target.generate_keypair();
    target = source;
    EXPECT_EQ(TestSecret{target.export_secret_key()}.get(), original_key.get());
    EXPECT_EQ(target.get_details().name, source.get_details().name);
    expect_sig_key_works(target, public_key);

    // Use an alias to exercise self-assignment without self-move warnings.
    auto& same_target = target;
    target = same_target;
    target = std::move(same_target);
    EXPECT_EQ(TestSecret{target.export_secret_key()}.get(), original_key.get());
    expect_sig_key_works(target, public_key);

    oqs::Signature moved{std::move(copied)};
    EXPECT_TRUE(TestSecret{copied.export_secret_key()}.get().empty());
    EXPECT_EQ(TestSecret{moved.export_secret_key()}.get(), original_key.get());
    expect_sig_key_works(moved, public_key);

    // Replace a different, already initialized key via move assignment.
    target = oqs::Signature{target_alg};
    target.generate_keypair();
    target = std::move(moved);
    EXPECT_TRUE(TestSecret{moved.export_secret_key()}.get().empty());
    EXPECT_EQ(TestSecret{target.export_secret_key()}.get(), original_key.get());
    expect_sig_key_works(target, public_key);

    const auto regenerated_public_key = target.generate_keypair();
    EXPECT_EQ(TestSecret{target.export_secret_key()}.get().size(),
              target.get_details().length_secret_key);
    expect_sig_key_works(target, regenerated_public_key);

    // Regenerating the destination must not change the source's copied key.
    EXPECT_EQ(TestSecret{source.export_secret_key()}.get(), original_key.get());
    expect_sig_key_works(source, public_key);

    // Moved-from wrappers remain assignable.
    copied = source;
    expect_sig_key_works(copied, public_key);
}

TEST(oqs_Signature, EmptySecretKeyAssignment) {
    const auto& enabled = oqs::Sigs::get_enabled_sigs();
    if (enabled.empty())
        GTEST_SKIP() << "No enabled SIG algorithms";
    const auto preferred = std::find(enabled.begin(), enabled.end(),
                                     "ML-DSA-65");
    const auto& alg = preferred != enabled.end() ? *preferred : enabled.front();
    SCOPED_TRACE(alg);

    oqs::Signature empty{alg};
    oqs::Signature copied_empty{empty};
    EXPECT_TRUE(TestSecret{copied_empty.export_secret_key()}.get().empty());

    oqs::Signature target{alg};
    target.generate_keypair();
    target = empty;
    EXPECT_TRUE(TestSecret{target.export_secret_key()}.get().empty());
    EXPECT_EQ(target.get_details().name, alg);

    target.generate_keypair();
    target = std::move(copied_empty);
    EXPECT_TRUE(TestSecret{target.export_secret_key()}.get().empty());
    EXPECT_TRUE(TestSecret{copied_empty.export_secret_key()}.get().empty());
    const auto public_key = target.generate_keypair();
    expect_sig_key_works(target, public_key);
}
