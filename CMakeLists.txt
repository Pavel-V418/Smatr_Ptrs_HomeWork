cmake_minimum_required(VERSION 3.20)
project(HM_SmatrPtrs)

set(CMAKE_CXX_STANDARD 20)

include(FetchContent)
FetchContent_Declare(
        googletest
        GIT_REPOSITORY https://github.com/google/googletest.git
        GIT_TAG        v1.14.0
)

# Предотвращает конфликты Runtime Library в Windows
set(gtest_force_shared_crt ON CACHE BOOL "" FORCE)
FetchContent_MakeAvailable(googletest)

add_executable(HM_SmatrPtrs main.cpp
        src/UnqPtr.h
        src/ShrdPtr.h
        src/MemorySpan.h
        src/MsPtr.h
)

add_executable(run_tests
        tests/unq_ptrs_tests.cpp
        tests/shrd_ptrs_tests.cpp
        tests/memory_span_tests.cpp
        tests/ms_ptrs_tests.cpp
)

# 5. Линковка тестов с GTest
target_link_libraries(run_tests GTest::gtest_main)

# 6. Интеграция с интерфейсом тестирования CLion
include(GoogleTest)
gtest_discover_tests(run_tests)