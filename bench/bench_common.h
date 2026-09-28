#pragma once
// Общая инфраструктура нагрузочных тестов: замер времени, отчёт
// в консоль и CSV. Конкретные сценарии для UnqPtr/ShrdPtr/MemorySpan/MsPtr
// лежат в bench_unqptr.cpp, bench_shrdptr.cpp, bench_memoryspan.cpp,
// bench_msptr.cpp и просто зовут run_scenario(...).

#include <chrono>
#include <fstream>
#include <iomanip>
#include <iostream>
#include <string>
#include <utility>
#include <vector>

#include "bench_alloc_counter.h"

struct BenchRow {
    std::string group;    // "UnqPtr", "ShrdPtr_ctor", "ShrdPtr_copy", "MemorySpan_build", ...
    std::string variant;  // "raw", "custom", "std", ...
    long long n = 0;
    double total_ms = 0.0;
    double ns_per_op = 0.0;
    long long bytes_total = 0; // суммарно выделено байт за сценарий (heap traffic)
    long long peak_bytes = 0;  // пиковая память сверх уровня до сценария
};

class BenchReport {
public:
    void add(BenchRow row) { rows_.push_back(std::move(row)); }

    void print_table() const {
        std::cout << std::left << std::setw(18) << "group" << std::setw(10) << "variant"
                   << std::right << std::setw(12) << "N" << std::setw(14) << "time_ms"
                   << std::setw(14) << "ns/op" << std::setw(16) << "bytes_total"
                   << std::setw(14) << "peak_bytes" << "\n";

        for (const auto& r : rows_) {
            std::cout << std::left << std::setw(18) << r.group << std::setw(10) << r.variant
                       << std::right << std::setw(12) << r.n << std::setw(14) << std::fixed
                       << std::setprecision(3) << r.total_ms << std::setw(14) << std::fixed
                       << std::setprecision(2) << r.ns_per_op << std::setw(16) << r.bytes_total
                       << std::setw(14) << r.peak_bytes << "\n";
        }
    }

    void write_csv(const std::string& path) const {
        std::ofstream out(path);
        out << "group,variant,n,time_ms,ns_per_op,bytes_total,peak_bytes\n";
        for (const auto& r : rows_) {
            out << r.group << ',' << r.variant << ',' << r.n << ',' << r.total_ms << ','
                << r.ns_per_op << ',' << r.bytes_total << ',' << r.peak_bytes << '\n';
        }
    }

private:
    std::vector<BenchRow> rows_;
};

// Время выполнения f() в миллисекундах.
template <class Func>
double measure_ms(Func&& f) {
    auto start = std::chrono::steady_clock::now();
    f();
    auto end = std::chrono::steady_clock::now();
    return std::chrono::duration<double, std::milli>(end - start).count();
}

// Прогоняет один сценарий: обнуляет счётчики аллокаций, измеряет время f(),
// считает память сверх уровня "до сценария" и кладёт готовую строку в отчёт.
template <class Func>
void run_scenario(BenchReport& report, const std::string& group, const std::string& variant,
                   long long n, Func&& f) {
    long long baseline = bench_alloc::reset();
    double ms = measure_ms(std::forward<Func>(f));
    auto snap = bench_alloc::snapshot();

    BenchRow row;
    row.group = group;
    row.variant = variant;
    row.n = n;
    row.total_ms = ms;
    row.ns_per_op = n > 0 ? (ms * 1e6) / static_cast<double>(n) : 0.0;
    row.bytes_total = snap.total_allocated_bytes;
    row.peak_bytes = snap.peak_bytes - baseline;
    report.add(std::move(row));
}
