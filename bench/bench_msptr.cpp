#include <vector>

#include "MemorySpan.h"
#include "MsPtr.h"
#include "bench_common.h"
#include "bench_escape.h"

namespace {
    inline int mix(int i) { return (i % 97) + 1; }
}

// Обход через MsPtr (locate() + operator++/operator*, оба с проверкой
// границ на каждый шаг) vs обход через сырой указатель без проверок —
// цена безопасности арифметики указателей у MsPtr.
void run_msptr_benchmarks(BenchReport& report, long long n) {
    int size = static_cast<int>(n);

    MemorySpan<int> span(size);
    for (int i = 0; i < size; ++i) span.set(i, mix(i));

    run_scenario(report, "MsPtr_traverse", "custom", n, [&span, size] {
        long long sum = 0;
        MsPtr<int> it = span.locate(0);
        for (int i = 0; i < size; ++i) {
            sum += *it;
            if (i + 1 < size) ++it;
        }
        bench_escape(&sum);
    });

    std::vector<int> raw(static_cast<std::size_t>(size));
    for (int i = 0; i < size; ++i) raw[static_cast<std::size_t>(i)] = mix(i);

    run_scenario(report, "MsPtr_traverse", "raw_pointer", n, [&raw, size] {
        long long sum = 0;
        const int* p = raw.data();
        for (int i = 0; i < size; ++i) sum += *(p + i);
        bench_escape(&sum);
    });
}
