// Unit testing oqs::KeyEncapsulation

#include <algorithm>
#include <exception>
#include <iostream>
#include <mutex>
#include <thread>
#include <tuple>
#include <utility>
#include <vector>

#include <gtest/gtest.h>

#include "oqs_cpp.hpp"
#include "rand/rand.hpp"

// KEMs that need the main thread's larger stack on some platforms.
static std::vector<std::string> no_thread_KEM_patterns{"Classic-McEliece",
                                                    "HQC-256"};
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

void test_kem_correctness(const std::string& kem_name) {
    {
        std::lock_guard<std::mutex> lg{mu};
        std::cout << "Correctness - " << kem_name << std::endl;
    }
    oqs::KeyEncapsulation client{kem_name};
    oqs::bytes client_public_key = client.generate_keypair();
    oqs::KeyEncapsulation server{kem_name};
    oqs::bytes ciphertext, shared_secret_server;
    std::tie(ciphertext, shared_secret_server) =
        server.encap_secret(client_public_key);
    TestSecret expected{std::move(shared_secret_server)};
    TestSecret actual{client.decap_secret(ciphertext)};
    EXPECT_EQ(actual.get(), expected.get());
}

void test_kem_wrong_ciphertext(const std::string& kem_name) {
    {
        std::lock_guard<std::mutex> lg{mu};
        std::cout << "Wrong ciphertext - " << kem_name << std::endl;
    }
    oqs::KeyEncapsulation client{kem_name};
    oqs::bytes client_public_key = client.generate_keypair();
    oqs::KeyEncapsulation server{kem_name};
    oqs::bytes ciphertext, shared_secret_server;
    std::tie(ciphertext, shared_secret_server) =
        server.encap_secret(client_public_key);
    TestSecret expected{std::move(shared_secret_server)};
    oqs::bytes wrong_ciphertext = oqs::rand::randombytes(ciphertext.size());
    try {
        // Accept either a decapsulation error or a different shared secret.
        TestSecret actual{client.decap_secret(wrong_ciphertext)};
        EXPECT_NE(actual.get(), expected.get());
    } catch (const std::exception& e) {
        if (std::string{e.what()} != "Can not decapsulate secret")
            throw;
    }
}

// An exception escaping a worker would terminate the entire test process.
void test_kem_in_thread(void (*test)(const std::string&),
                        const std::string& kem_name) {
    try {
        test(kem_name);
    } catch (const std::exception& e) {
        ADD_FAILURE() << kem_name << ": unexpected exception: " << e.what();
    } catch (...) {
        ADD_FAILURE() << kem_name << ": unexpected exception";
    }
}

TEST(oqs_KeyEncapsulation, Correctness) {
    const auto& enabled = oqs::KEMs::get_enabled_KEMs();
    if (enabled.empty())
        GTEST_SKIP() << "No enabled KEM algorithms";

    std::vector<std::thread> thread_pool;
    // Run algorithms requiring a larger stack on the main thread first.
    for (const auto& alg : enabled) {
        for (const auto& pattern : no_thread_KEM_patterns) {
            if (alg.find(pattern) != std::string::npos) {
                test_kem_correctness(alg);
                break;
            }
        }
    }
    for (const auto& alg : enabled) {
        bool test_in_thread = true;
        for (const auto& pattern : no_thread_KEM_patterns) {
            if (alg.find(pattern) != std::string::npos) {
                test_in_thread = false;
                break;
            }
        }
        if (test_in_thread)
            thread_pool.emplace_back(test_kem_in_thread,
                                     test_kem_correctness, alg);
    }
    for (auto& thread : thread_pool)
        thread.join();
}

TEST(oqs_KeyEncapsulation, WrongCiphertext) {
    const auto& enabled = oqs::KEMs::get_enabled_KEMs();
    if (enabled.empty())
        GTEST_SKIP() << "No enabled KEM algorithms";

    std::vector<std::thread> thread_pool;
    // Run algorithms requiring a larger stack on the main thread first.
    for (const auto& alg : enabled) {
        for (const auto& pattern : no_thread_KEM_patterns) {
            if (alg.find(pattern) != std::string::npos) {
                test_kem_wrong_ciphertext(alg);
                break;
            }
        }
    }
    for (const auto& alg : enabled) {
        bool test_in_thread = true;
        for (const auto& pattern : no_thread_KEM_patterns) {
            if (alg.find(pattern) != std::string::npos) {
                test_in_thread = false;
                break;
            }
        }
        if (test_in_thread)
            thread_pool.emplace_back(test_kem_in_thread,
                                     test_kem_wrong_ciphertext, alg);
    }
    for (auto& thread : thread_pool)
        thread.join();
}

namespace {
void expect_kem_key_works(const oqs::KeyEncapsulation& key,
                          const oqs::bytes& public_key) {
    oqs::KeyEncapsulation peer{key.get_details().name};
    auto encapsulated = peer.encap_secret(public_key);
    TestSecret expected{std::move(encapsulated.second)};
    TestSecret actual{key.decap_secret(encapsulated.first)};
    EXPECT_EQ(actual.get(), expected.get());
}
} // namespace

TEST(oqs_KeyEncapsulation, NotSupported) {
    EXPECT_THROW(oqs::KeyEncapsulation{"unsupported_kem"},
                 oqs::MechanismNotSupportedError);
}

TEST(oqs_KeyEncapsulation, NotSupportedWithSecretKey) {
    // This checks the exception and preservation of a caller-owned lvalue.
    // Cleansing on failure is tested separately by the instrumented harness.
    const TestSecret caller_key{oqs::bytes(32, 0xa5)};
    EXPECT_THROW((oqs::KeyEncapsulation{"unsupported_kem", caller_key.get()}),
                 oqs::MechanismNotSupportedError);
    EXPECT_TRUE(std::all_of(caller_key.get().begin(), caller_key.get().end(),
                            [](unsigned char byte) { return byte == 0xa5; }));
}

TEST(oqs_KeyEncapsulation, SecretKeyLifecycle) {
    const auto& enabled = oqs::KEMs::get_enabled_KEMs();
    if (enabled.empty())
        GTEST_SKIP() << "No enabled KEM algorithms";

    // Prefer a fast standard algorithm for repeated lifecycle operations.
    // Run this test on the main thread, including the fallback algorithm.
    const auto preferred = std::find(enabled.begin(), enabled.end(),
                                     "ML-KEM-768");
    const auto& alg = preferred != enabled.end() ? *preferred : enabled.front();
    SCOPED_TRACE(alg);

    oqs::KeyEncapsulation source{alg};
    const auto public_key = source.generate_keypair();
    const TestSecret original_key{source.export_secret_key()};
    ASSERT_EQ(original_key.get().size(), source.get_details().length_secret_key);

    // Copy construction preserves the source and yields a usable key.
    oqs::KeyEncapsulation copied{source};
    EXPECT_EQ(TestSecret{copied.export_secret_key()}.get(), original_key.get());
    expect_kem_key_works(copied, public_key);

    // Exercise replacement across algorithms when the alternative is enabled.
    const auto alternative = std::find(enabled.begin(), enabled.end(),
                                       "ML-KEM-512");
    const auto& target_alg = alternative != enabled.end() ? *alternative : alg;
    oqs::KeyEncapsulation target{target_alg};
    target.generate_keypair();
    target = source;
    EXPECT_EQ(TestSecret{target.export_secret_key()}.get(), original_key.get());
    EXPECT_EQ(target.get_details().name, source.get_details().name);
    expect_kem_key_works(target, public_key);

    // Use an alias to exercise self-assignment without self-move warnings.
    auto& same_target = target;
    target = same_target;
    target = std::move(same_target);
    EXPECT_EQ(TestSecret{target.export_secret_key()}.get(), original_key.get());
    expect_kem_key_works(target, public_key);

    oqs::KeyEncapsulation moved{std::move(copied)};
    EXPECT_TRUE(TestSecret{copied.export_secret_key()}.get().empty());
    EXPECT_EQ(TestSecret{moved.export_secret_key()}.get(), original_key.get());
    expect_kem_key_works(moved, public_key);

    // Replace a different, already initialized key via move assignment.
    target = oqs::KeyEncapsulation{target_alg};
    target.generate_keypair();
    target = std::move(moved);
    EXPECT_TRUE(TestSecret{moved.export_secret_key()}.get().empty());
    EXPECT_EQ(TestSecret{target.export_secret_key()}.get(), original_key.get());
    expect_kem_key_works(target, public_key);

    const auto regenerated_public_key = target.generate_keypair();
    EXPECT_EQ(TestSecret{target.export_secret_key()}.get().size(),
              target.get_details().length_secret_key);
    expect_kem_key_works(target, regenerated_public_key);

    // Regenerating the destination must not change the source's copied key.
    EXPECT_EQ(TestSecret{source.export_secret_key()}.get(), original_key.get());
    expect_kem_key_works(source, public_key);

    // Moved-from wrappers remain assignable.
    copied = source;
    expect_kem_key_works(copied, public_key);
}

TEST(oqs_KeyEncapsulation, EmptySecretKeyAssignment) {
    const auto& enabled = oqs::KEMs::get_enabled_KEMs();
    if (enabled.empty())
        GTEST_SKIP() << "No enabled KEM algorithms";
    const auto preferred = std::find(enabled.begin(), enabled.end(),
                                     "ML-KEM-768");
    const auto& alg = preferred != enabled.end() ? *preferred : enabled.front();
    SCOPED_TRACE(alg);

    oqs::KeyEncapsulation empty{alg};
    oqs::KeyEncapsulation copied_empty{empty};
    EXPECT_TRUE(TestSecret{copied_empty.export_secret_key()}.get().empty());

    oqs::KeyEncapsulation target{alg};
    target.generate_keypair();
    target = empty;
    EXPECT_TRUE(TestSecret{target.export_secret_key()}.get().empty());
    EXPECT_EQ(target.get_details().name, alg);

    target.generate_keypair();
    target = std::move(copied_empty);
    EXPECT_TRUE(TestSecret{target.export_secret_key()}.get().empty());
    EXPECT_TRUE(TestSecret{copied_empty.export_secret_key()}.get().empty());
    const auto public_key = target.generate_keypair();
    expect_kem_key_works(target, public_key);
}
