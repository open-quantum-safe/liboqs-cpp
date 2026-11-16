// Test-only hooks on the global allocation functions, used to check how
// oqs::KeyEncapsulation and oqs::Signature manage secret-key storage.
//
// alloc_hooks.cpp replaces the global operator new and operator delete for the
// whole unit_tests executable. When no hook is active they forward to malloc
// and free. Hooks are not thread-safe; use them only from single-threaded
// tests. At most one hook of each type may be active at a time.

#ifndef ALLOC_HOOKS_HPP
#define ALLOC_HOOKS_HPP

#include <cstddef>
#include <cstdint>
#include <vector>

namespace alloc_hooks {

/// True if freed heap blocks can be inspected on this platform, which
/// FreedSecretWatch requires.
bool can_inspect_freed_blocks();

/// While alive, searches each heap block released through the global operator
/// delete for a copy of the watched secret. The secret's own storage must
/// outlive the watch and must not be released while the watch is active.
class FreedSecretWatch {
  public:
    explicit FreedSecretWatch(const std::vector<std::uint8_t>& secret);
    ~FreedSecretWatch();
    FreedSecretWatch(const FreedSecretWatch&) = delete;
    FreedSecretWatch& operator=(const FreedSecretWatch&) = delete;

    /// True if a released block still held the secret, i.e., it was freed
    /// without being cleansed.
    bool secret_was_freed() const;
};

/// While alive, counts calls to the global operator new for exactly `size`
/// bytes.
class AllocationCounter {
  public:
    explicit AllocationCounter(std::size_t size);
    ~AllocationCounter();
    AllocationCounter(const AllocationCounter&) = delete;
    AllocationCounter& operator=(const AllocationCounter&) = delete;

    std::size_t count() const;
};

/// While alive, makes the n-th following call to the global operator new
/// (counting from 1) throw std::bad_alloc.
class AllocationFailure {
  public:
    explicit AllocationFailure(std::size_t n);
    ~AllocationFailure();
    AllocationFailure(const AllocationFailure&) = delete;
    AllocationFailure& operator=(const AllocationFailure&) = delete;
};

} // namespace alloc_hooks

#endif // ALLOC_HOOKS_HPP
