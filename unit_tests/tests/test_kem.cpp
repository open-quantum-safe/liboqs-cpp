// Unit testing oqs::KeyEncapsulation

#include <algorithm>
#include <iostream>
#include <mutex>
#include <new>
#include <string>
#include <thread>
#include <tuple>
#include <utility>
#include <vector>

#include <gtest/gtest.h>

#include "oqs_cpp.hpp"
#include "rand/rand.hpp"

#include "alloc_hooks.hpp"

// no_thread_KEM_patterns lists KEM patterns that have issues running in a
// separate thread
static std::vector<std::string> no_thread_KEM_patterns{"Classic-McEliece",
                                                       "HQC-256"};

// used for thread-safe console output
static std::mutex mu;

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
    oqs::bytes shared_secret_client = client.decap_secret(ciphertext);
    bool is_valid = (shared_secret_client == shared_secret_server);
    if (!is_valid)
        std::cerr << kem_name << ": shared secrets do not coincide"
                  << std::endl;
    EXPECT_TRUE(is_valid);
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
    oqs::bytes wrong_ciphertext = oqs::rand::randombytes(ciphertext.size());
    oqs::bytes shared_secret_client;
    try {
        // this line should throw
        shared_secret_client = client.decap_secret(wrong_ciphertext);
        // if not, test should fail
        bool is_valid = (shared_secret_client == shared_secret_server);
        if (is_valid)
            std::cerr << kem_name << ": shared secrets should not coincide"
                      << std::endl;
        EXPECT_FALSE(is_valid);
    } catch (std::exception& e) {
        if (e.what() == std::string{"Can not decapsulate secret"})
            return;
        else
            throw; // this is another un-expected exception
    }
}

TEST(oqs_KeyEncapsulation, Correctness) {
    std::vector<std::thread> thread_pool;
    std::vector<std::string> enabled_KEMs = oqs::KEMs::get_enabled_KEMs();
    // first test KEMs that belong to no_thread_KEM_patterns[] in the main
    // thread (stack size is 8Mb on macOS), due to issues with stack size being
    // too small in macOS (512Kb for threads)
    for (auto&& kem_name : enabled_KEMs) {
        for (auto&& no_thread_kem : no_thread_KEM_patterns) {
            if (kem_name.find(no_thread_kem) != std::string::npos) {
                test_kem_correctness(kem_name);
            }
        }
    }
    // test the remaining KEMs in separate threads
    for (auto&& kem_name : enabled_KEMs) {
        bool test_in_thread = true;
        for (auto&& no_thread_kem : no_thread_KEM_patterns) {
            if (kem_name.find(no_thread_kem) != std::string::npos) {
                test_in_thread = false;
                break;
            }
        }
        if (test_in_thread)
            thread_pool.emplace_back(test_kem_correctness, kem_name);
    }
    // join the rest of the threads
    for (auto&& elem : thread_pool)
        elem.join();
}

TEST(oqs_KeyEncapsulation, WrongCiphertext) {
    std::vector<std::thread> thread_pool;
    std::vector<std::string> enabled_KEMs = oqs::KEMs::get_enabled_KEMs();
    // first test KEMs that belong to no_thread_KEM_patterns[] in the main
    // thread (stack size is 8Mb on macOS), due to issues with stack size being
    // too small in macOS (512Kb for threads)
    for (auto&& kem_name : enabled_KEMs) {
        for (auto&& no_thread_kem : no_thread_KEM_patterns) {
            if (kem_name.find(no_thread_kem) != std::string::npos) {
                test_kem_wrong_ciphertext(kem_name);
            }
        }
    }
    // test the remaining KEMs in separate threads
    for (auto&& kem_name : enabled_KEMs) {
        bool test_in_thread = true;
        for (auto&& no_thread_kem : no_thread_KEM_patterns) {
            if (kem_name.find(no_thread_kem) != std::string::npos) {
                test_in_thread = false;
                break;
            }
        }
        if (test_in_thread)
            thread_pool.emplace_back(test_kem_wrong_ciphertext, kem_name);
    }
    // join the rest of the threads
    for (auto&& elem : thread_pool)
        elem.join();
}

TEST(oqs_KeyEncapsulation, NotSupported) {
    EXPECT_THROW(oqs::KeyEncapsulation{"unsupported_kem"},
                 oqs::MechanismNotSupportedError);
}

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

namespace {
// Returns the KEM for the secret-key tests: ML-KEM-768 if enabled (it is fast
// and standard), otherwise the first enabled KEM, or "" if none is enabled.
std::string kem_for_secret_key_tests() {
    const auto& enabled = oqs::KEMs::get_enabled_KEMs();
    if (enabled.empty())
        return {};
    const auto preferred =
        std::find(enabled.begin(), enabled.end(), "ML-KEM-768");
    return preferred != enabled.end() ? *preferred : enabled.front();
}
} // namespace

TEST(oqs_KeyEncapsulation, FailedConstructionCleansesSecretKey) {
    if (!alloc_hooks::can_inspect_freed_blocks())
        GTEST_SKIP() << "Freed heap blocks cannot be inspected here";

    // The constructor takes its own copy of the key, which it must cleanse
    // before releasing when it throws.
    const TestSecret key{oqs::rand::randombytes(64)};
    alloc_hooks::FreedSecretWatch watch{key.get()};
    EXPECT_THROW((oqs::KeyEncapsulation{"unsupported_kem", key.get()}),
                 oqs::MechanismNotSupportedError);
    EXPECT_FALSE(watch.secret_was_freed());
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

TEST(oqs_KeyEncapsulation, RegeneratingCleansesReplacedSecretKey) {
    if (!alloc_hooks::can_inspect_freed_blocks())
        GTEST_SKIP() << "Freed heap blocks cannot be inspected here";
    const std::string alg = kem_for_secret_key_tests();
    if (alg.empty())
        GTEST_SKIP() << "No enabled KEM algorithms";
    SCOPED_TRACE(alg);

    oqs::KeyEncapsulation kem{alg};
    kem.generate_keypair();
    const TestSecret old_key{kem.export_secret_key()};
    alloc_hooks::FreedSecretWatch watch{old_key.get()};
    kem.generate_keypair();
    EXPECT_FALSE(watch.secret_was_freed());
}

TEST(oqs_KeyEncapsulation, AssignmentCleansesReplacedSecretKey) {
    if (!alloc_hooks::can_inspect_freed_blocks())
        GTEST_SKIP() << "Freed heap blocks cannot be inspected here";
    const std::string alg = kem_for_secret_key_tests();
    if (alg.empty())
        GTEST_SKIP() << "No enabled KEM algorithms";
    SCOPED_TRACE(alg);

    oqs::KeyEncapsulation keyed{alg};
    keyed.generate_keypair();
    const TestSecret old_key{keyed.export_secret_key()};
    const oqs::KeyEncapsulation empty{alg};

    // Assigning from a wrapper without a key leaves the old key nothing to be
    // overwritten by. Destroy the target inside the watch, so that storage
    // released after the assignment is also inspected.
    {
        alloc_hooks::FreedSecretWatch watch{old_key.get()};
        {
            oqs::KeyEncapsulation target{alg, old_key.get()};
            target = empty;
        }
        EXPECT_FALSE(watch.secret_was_freed()) << "copy assignment";
    }
    {
        alloc_hooks::FreedSecretWatch watch{old_key.get()};
        {
            oqs::KeyEncapsulation target{alg, old_key.get()};
            target = oqs::KeyEncapsulation{alg};
        }
        EXPECT_FALSE(watch.secret_was_freed()) << "move assignment";
    }
}

TEST(oqs_KeyEncapsulation, MovesDoNotCopySecretKey) {
    const std::string alg = kem_for_secret_key_tests();
    if (alg.empty())
        GTEST_SKIP() << "No enabled KEM algorithms";
    SCOPED_TRACE(alg);

    oqs::KeyEncapsulation source{alg};
    source.generate_keypair();
    oqs::KeyEncapsulation target{alg};

    // Moves are noexcept, so copying the key there would terminate the
    // process if the allocation failed.
    alloc_hooks::AllocationCounter key_allocations{
        source.get_details().length_secret_key};
    oqs::KeyEncapsulation moved{std::move(source)};
    EXPECT_EQ(key_allocations.count(), 0u) << "move construction";
    target = std::move(moved);
    EXPECT_EQ(key_allocations.count(), 0u) << "move assignment";
}

TEST(oqs_KeyEncapsulation, FailedCopyAssignmentKeepsDestination) {
    // The source key is larger than the destination's, so copying it always
    // allocates, even if the destination's storage were reused.
    const std::string small_alg = "ML-KEM-512";
    const std::string large_alg = "ML-KEM-768";
    if (!oqs::KEMs::is_KEM_enabled(small_alg) ||
        !oqs::KEMs::is_KEM_enabled(large_alg))
        GTEST_SKIP() << small_alg << " and " << large_alg
                     << " must both be enabled";

    oqs::KeyEncapsulation target{small_alg};
    target.generate_keypair();
    const TestSecret target_key{target.export_secret_key()};
    oqs::KeyEncapsulation source{large_alg};
    source.generate_keypair();

    // Fail each allocation made by the assignment in turn, until it succeeds.
    std::size_t failures = 0;
    for (std::size_t n = 1; n <= 100; ++n) {
        bool threw = false;
        {
            alloc_hooks::AllocationFailure fail{n};
            try {
                target = source;
            } catch (const std::bad_alloc&) {
                threw = true;
            }
        }
        if (!threw)
            break;
        ++failures;
        EXPECT_EQ(target.get_details().name, small_alg) << "allocation " << n;
        EXPECT_TRUE(TestSecret{target.export_secret_key()}.get() ==
                    target_key.get())
            << "allocation " << n;
    }
    EXPECT_GT(failures, 0u);
    EXPECT_EQ(target.get_details().name, large_alg);
}
