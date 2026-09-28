#pragma once
// счётчик аллокаций.
// Переопределяет глобальные operator new/delete на весь бинарник run_benchmarks
// и считает суммарные и пиковые байты.

#include <cstddef>

namespace bench_alloc {

    struct Snapshot {
        long long total_allocated_bytes = 0; // суммарно запрошено байт с последнего reset()
        long long total_allocations = 0;     // количество вызовов new/new[] с последнего reset()
        long long current_bytes = 0;         // сколько байт живо прямо сейчас
        long long peak_bytes = 0;            // максимум current_bytes с последнего reset()
    };

    // Снимок счётчиков на текущий момент.
    Snapshot snapshot();

    // Обнуляет total_allocated_bytes/total_allocations, взводит peak_bytes
    // равным текущему уровню (чтобы пик считался от него, а не от нуля —
    // иначе в peak_bytes попадала бы память, живущая ещё до сценария:
    // буферы iostream, ранее созданные объекты и т.п.).
    long long reset();

} // namespace bench_alloc
