#include <memory>

#include "ShrdPtr.h"
#include "bench_common.h"
#include "bench_escape.h"

// ShrdPtr<T> vs сырой new/delete vs std::shared_ptr<T>.
//
// Два разных по смыслу сценария:
//  - _ctor  — создание владельца "с нуля" (важно: ShrdPtr выделяет счётчик
//             ссылок даже для нового указателя, т.е. это ВСЕГДА минимум
//             две аллокации на объект — сам объект + int-счётчик; поэтому
//             std::shared_ptr здесь тоже строится через new, а не через
//             make_shared — тот делает одну аллокацию и был бы другой
//             операцией, нечестной для сравнения);
//  - _copy  — стоимость самого разделяемого владения: инкремент/декремент
//             счётчика ссылок при копировании уже существующего указателя.
//             У сырого указателя такой операции по смыслу нет (копия — это
//             просто копия адреса без всякой бухгалтерии), поэтому "raw"
//             вариант здесь не участвует — сравнивать было бы нечестно.
void run_shrdptr_benchmarks(BenchReport& report, long long n) {
    run_scenario(report, "ShrdPtr_ctor", "raw", n, [n] {
        for (long long i = 0; i < n; ++i) {
            int* p = new int(static_cast<int>(i % 97));
            bench_escape(&*p);
            delete p;
        }
    });

    run_scenario(report, "ShrdPtr_ctor", "custom", n, [n] {
        for (long long i = 0; i < n; ++i) {
            ShrdPtr<int> p(new int(static_cast<int>(i % 97)));
            bench_escape(&*p);
        }
    });

    run_scenario(report, "ShrdPtr_ctor", "std", n, [n] {
        for (long long i = 0; i < n; ++i) {
            std::shared_ptr<int> p(new int(static_cast<int>(i % 97)));
            bench_escape(&*p);
        }
    });

    {
        ShrdPtr<int> original(new int(7));
        run_scenario(report, "ShrdPtr_copy", "custom", n, [&original, n] {
            for (long long i = 0; i < n; ++i) {
                ShrdPtr<int> copy(original);
                bench_escape(&*copy);
            }
        });
    }
    {
        std::shared_ptr<int> original = std::make_shared<int>(7);
        run_scenario(report, "ShrdPtr_copy", "std", n, [&original, n] {
            for (long long i = 0; i < n; ++i) {
                std::shared_ptr<int> copy(original);
                bench_escape(&*copy);
            }
        });
    }
}
