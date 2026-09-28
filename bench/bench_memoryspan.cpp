#include <vector>

#include "MemorySpan.h"
#include "bench_common.h"
#include "bench_escape.h"

namespace {
    // Не арифметическая прогрессия i, а i % 97 — чтобы сумма элементов не
    // сворачивалась компилятором в формулу треугольного числа n(n-1)/2.
    inline int mix(int i) { return (i % 97) + 1; }
}

// MemorySpan<T> vs сырой new[]/delete[] vs std::vector<T>.
//
// _build — конструирование + заполнение через set() (сам set() ничего не
//          выделяет, только проверяет границы и присваивает — должен быть
//          близок к std::vector по накладным расходам).
// _read  — поэлементное чтение. Важный нюанс их API: MemorySpan::get(index)
//          возвращает UnqPtr<T>(new T(data[index])) — то есть КАЖДОЕ чтение
//          элемента делает копию значения в новую кучевую аллокацию. Это не
//          "просто индексация", а по сути (T)-значение, упакованное в
//          владеющий указатель. Сравните bytes_total у custom_get и
//          raw_index в итоговой таблице/графике — это и есть цена такого
//          дизайна get().
void run_memoryspan_benchmarks(BenchReport& report, long long n) {
    int size = static_cast<int>(n);

    run_scenario(report, "MemorySpan_build", "raw_array", n, [size] {
        int* arr = new int[static_cast<std::size_t>(size)];
        for (int i = 0; i < size; ++i) arr[i] = mix(i);
        bench_escape(arr);
        delete[] arr;
    });

    run_scenario(report, "MemorySpan_build", "custom", n, [size] {
        MemorySpan<int> span(size);
        for (int i = 0; i < size; ++i) span.set(i, mix(i));
        bench_escape(&span);
    });

    run_scenario(report, "MemorySpan_build", "std_vector", n, [size] {
        std::vector<int> v(static_cast<std::size_t>(size));
        for (int i = 0; i < size; ++i) v[static_cast<std::size_t>(i)] = mix(i);
        bench_escape(v.data());
    });

    {
        MemorySpan<int> span(size);
        for (int i = 0; i < size; ++i) span.set(i, mix(i));

        run_scenario(report, "MemorySpan_read", "custom_get", n, [&span, size] {
            long long sum = 0;
            for (int i = 0; i < size; ++i) {
                UnqPtr<int> v = span.get(i); // выделяет новый int на каждый вызов
                // Без escape внутри цикла компилятор вправе применить elision
                // пары new/delete для временного объекта (стандарт это прямо
                // разрешает) и вообще убрать аллокацию — тогда bytes_total
                // соврёт, показав 0 там, где реально N аллокаций.
                bench_escape(&*v);
                sum += *v;
            }
            bench_escape(&sum);
        });
    }
    {
        std::vector<int> v(static_cast<std::size_t>(size));
        for (int i = 0; i < size; ++i) v[static_cast<std::size_t>(i)] = mix(i);

        run_scenario(report, "MemorySpan_read", "raw_index", n, [&v, size] {
            long long sum = 0;
            for (int i = 0; i < size; ++i) sum += v[static_cast<std::size_t>(i)];
            bench_escape(&sum);
        });
    }
}
