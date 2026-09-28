#include "bench_escape.h"

namespace {
    volatile const void* g_escape_sink = nullptr;
}

void bench_escape(const void* p) {
    g_escape_sink = p;
}
