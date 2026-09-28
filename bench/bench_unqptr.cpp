#include <memory>

#include "UnqPtr.h"
#include "bench_common.h"
#include "bench_escape.h"

// UnqPtr<T> vs сырой new/delete vs std::unique_ptr<T>:
// стоимость связки "выделить объект + обернуть в умный указатель + разрушить".

// Значение под указателем берём из i (i % 97), а не константу: иначе при
// -O2 компилятор может доказать, что результат всегда один и тот же, и
// свернуть цикл в замкнутую форму, а не выполнить n реальных аллокаций.
void run_unqptr_benchmarks(BenchReport& report, long long n) {
    run_scenario(report, "UnqPtr", "raw", n, [n] {
        for (long long i = 0; i < n; ++i) {
            int* p = new int(static_cast<int>(i % 97));
            bench_escape(&*p);
            delete p;
        }
    });

    run_scenario(report, "UnqPtr", "custom", n, [n] {
        for (long long i = 0; i < n; ++i) {
            UnqPtr<int> p(new int(static_cast<int>(i % 97)));
            bench_escape(&*p);
        }
    });

    run_scenario(report, "UnqPtr", "std", n, [n] {
        for (long long i = 0; i < n; ++i) {
            std::unique_ptr<int> p(new int(static_cast<int>(i % 97)));
            bench_escape(&*p);
        }
    });
}
