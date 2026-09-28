#include <cstdlib>
#include <string>
#include <vector>

#include "bench_common.h"

void run_unqptr_benchmarks(BenchReport&, long long n);
void run_shrdptr_benchmarks(BenchReport&, long long n);
void run_memoryspan_benchmarks(BenchReport&, long long n);
void run_msptr_benchmarks(BenchReport&, long long n);

int main(int argc, char** argv) {
    // По ТЗ: малое число объектов (единицы-тысячи) и большое (10^6-10^8).
    // 10^8 по умолчанию не гоняем — это заметно по времени и по памяти на
    // слабых машинах; передайте свой размер флагом:
    //   run_benchmarks --big 100000000
    std::vector<long long> sizes = {10, 100, 1'000, 1'000'000, 10'000'000};

    for (int i = 1; i < argc; ++i) {
        if (std::string(argv[i]) == "--big" && i + 1 < argc) {
            sizes.push_back(std::atoll(argv[i + 1]));
            ++i;
        }
    }

    BenchReport report;
    for (long long n : sizes) {
        run_unqptr_benchmarks(report, n);
        run_shrdptr_benchmarks(report, n);
        run_memoryspan_benchmarks(report, n);
        run_msptr_benchmarks(report, n);
    }

    report.print_table();
    report.write_csv("bench_results.csv");
    std::cout << "\nCSV: bench_results.csv (in cmake-build-debug)\n";
    return 0;
}
