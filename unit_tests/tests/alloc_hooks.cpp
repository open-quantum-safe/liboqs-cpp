// Replacements for the global operator new and operator delete that support
// the hooks declared in alloc_hooks.hpp

#include "alloc_hooks.hpp"

#include <algorithm>
#include <atomic>
#include <cstdlib>
#include <new>

#if defined(__APPLE__)
#include <malloc/malloc.h>
#elif defined(_WIN32) || defined(__linux__)
#include <malloc.h>
#endif

namespace {

#if defined(__APPLE__)
constexpr bool inspection_supported = true;
std::size_t block_size(void* p) { return malloc_size(p); }
#elif defined(_WIN32)
constexpr bool inspection_supported = true;
std::size_t block_size(void* p) { return _msize(p); }
#elif defined(__linux__)
constexpr bool inspection_supported = true;
std::size_t block_size(void* p) { return malloc_usable_size(p); }
#else
constexpr bool inspection_supported = false;
std::size_t block_size(void*) { return 0; }
#endif

// All state is constant-initialized, so it is usable during static
// initialization and destruction.
std::atomic<const unsigned char*> watched_data{nullptr};
std::atomic<std::size_t> watched_size{0};
std::atomic<bool> watched_found{false};

std::atomic<bool> counting{false};
std::atomic<std::size_t> counted_size{0};
std::atomic<std::size_t> counted{0};

std::atomic<std::size_t> fail_countdown{0}; // 0 means disabled

void inspect_before_free(void* p) noexcept {
    const unsigned char* needle = watched_data.load(std::memory_order_acquire);
    if (needle == nullptr)
        return;
    const auto* begin = static_cast<const unsigned char*>(p);
    const auto* end = begin + block_size(p);
    const auto* needle_end =
        needle + watched_size.load(std::memory_order_relaxed);
    if (std::search(begin, end, needle, needle_end) != end)
        watched_found.store(true, std::memory_order_relaxed);
}

} // namespace

namespace alloc_hooks {

bool can_inspect_freed_blocks() { return inspection_supported; }

FreedSecretWatch::FreedSecretWatch(const std::vector<std::uint8_t>& secret) {
    watched_found.store(false, std::memory_order_relaxed);
    watched_size.store(secret.size(), std::memory_order_relaxed);
    // An empty needle would match every block, so leave the watch disarmed.
    if (!secret.empty())
        watched_data.store(secret.data(), std::memory_order_release);
}

FreedSecretWatch::~FreedSecretWatch() {
    watched_data.store(nullptr, std::memory_order_release);
}

bool FreedSecretWatch::secret_was_freed() const {
    return watched_found.load(std::memory_order_relaxed);
}

AllocationCounter::AllocationCounter(std::size_t size) {
    counted.store(0, std::memory_order_relaxed);
    counted_size.store(size, std::memory_order_relaxed);
    counting.store(true, std::memory_order_release);
}

AllocationCounter::~AllocationCounter() {
    counting.store(false, std::memory_order_release);
}

std::size_t AllocationCounter::count() const {
    return counted.load(std::memory_order_relaxed);
}

AllocationFailure::AllocationFailure(std::size_t n) {
    fail_countdown.store(n, std::memory_order_release);
}

AllocationFailure::~AllocationFailure() {
    fail_countdown.store(0, std::memory_order_release);
}

} // namespace alloc_hooks

// The other replaceable forms (array, nothrow and, where applicable, sized)
// forward to these two by default. Over-aligned forms are separate and are not
// hooked.
void* operator new(std::size_t size) {
    const std::size_t remaining =
        fail_countdown.load(std::memory_order_acquire);
    if (remaining != 0) {
        fail_countdown.store(remaining - 1, std::memory_order_release);
        if (remaining == 1)
            throw std::bad_alloc{};
    }
    if (counting.load(std::memory_order_acquire) &&
        size == counted_size.load(std::memory_order_relaxed))
        counted.fetch_add(1, std::memory_order_relaxed);

    if (size == 0)
        size = 1;
    for (;;) {
        if (void* p = std::malloc(size))
            return p;
        std::new_handler handler = std::get_new_handler();
        if (handler == nullptr)
            throw std::bad_alloc{};
        handler();
    }
}

void operator delete(void* p) noexcept {
    if (p == nullptr)
        return;
    inspect_before_free(p);
    std::free(p);
}

#if defined(__cpp_sized_deallocation)
void operator delete(void* p, std::size_t) noexcept { ::operator delete(p); }
#endif
