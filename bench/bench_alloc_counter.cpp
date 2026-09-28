#include "bench_alloc_counter.h"

#include <atomic>
#include <cstdlib>
#include <new>

namespace {

    std::atomic<long long> g_total_allocated{0};
    std::atomic<long long> g_total_allocations{0};
    std::atomic<long long> g_current_bytes{0};
    std::atomic<long long> g_peak_bytes{0};

    constexpr std::size_t kAlign = alignof(std::max_align_t);

    // Заголовок перед пользовательскими данными: хранит исходный размер,
    // чтобы operator delete(void*) без параметра size тоже мог его учесть
    // (sized deallocation в C++ не гарантированно вызывается компилятором).
    struct Header {
        std::size_t size;
    };

    constexpr std::size_t header_span() {
        return ((sizeof(Header) + kAlign - 1) / kAlign) * kAlign;
    }

    void* raw_alloc(std::size_t size) {
        void* base = std::malloc(header_span() + size);
        if (!base) throw std::bad_alloc();
        reinterpret_cast<Header*>(base)->size = size;

        g_total_allocated.fetch_add(static_cast<long long>(size), std::memory_order_relaxed);
        g_total_allocations.fetch_add(1, std::memory_order_relaxed);

        long long now = g_current_bytes.fetch_add(static_cast<long long>(size),
                                                    std::memory_order_relaxed)
                         + static_cast<long long>(size);
        long long prev_peak = g_peak_bytes.load(std::memory_order_relaxed);
        while (now > prev_peak &&
               !g_peak_bytes.compare_exchange_weak(prev_peak, now, std::memory_order_relaxed)) {
        }

        return static_cast<char*>(base) + header_span();
    }

    void raw_free(void* p) noexcept {
        if (!p) return;
        void* base = static_cast<char*>(p) - header_span();
        std::size_t size = reinterpret_cast<Header*>(base)->size;
        g_current_bytes.fetch_sub(static_cast<long long>(size), std::memory_order_relaxed);
        std::free(base);
    }

} // namespace

void* operator new(std::size_t size) { return raw_alloc(size); }
void* operator new[](std::size_t size) { return raw_alloc(size); }
void operator delete(void* p) noexcept { raw_free(p); }
void operator delete[](void* p) noexcept { raw_free(p); }
void operator delete(void* p, std::size_t) noexcept { raw_free(p); }
void operator delete[](void* p, std::size_t) noexcept { raw_free(p); }

namespace bench_alloc {

    Snapshot snapshot() {
        Snapshot s;
        s.total_allocated_bytes = g_total_allocated.load(std::memory_order_relaxed);
        s.total_allocations = g_total_allocations.load(std::memory_order_relaxed);
        s.current_bytes = g_current_bytes.load(std::memory_order_relaxed);
        s.peak_bytes = g_peak_bytes.load(std::memory_order_relaxed);
        return s;
    }

    long long reset() {
        long long baseline = g_current_bytes.load(std::memory_order_relaxed);
        g_total_allocated.store(0, std::memory_order_relaxed);
        g_total_allocations.store(0, std::memory_order_relaxed);
        g_peak_bytes.store(baseline, std::memory_order_relaxed);
        return baseline;
    }

} // namespace bench_alloc
