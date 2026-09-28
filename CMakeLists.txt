cmake_minimum_required(VERSION 3.20)
project(HM_SmatrPtrs)

set(CMAKE_CXX_STANDARD 20)

enable_testing()

option(ENABLE_SANITIZERS "Собирать run_tests с AddressSanitizer + UndefinedBehaviorSanitizer" OFF)

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
        tests/instance_tracker.h
        tests/instance_tracker.cpp
        tests/test_main.cpp
)

# 5. Линковка тестов с GTest
# Используем GTest::gtest (без встроенного main), т.к. свой main.cpp
# в tests/test_main.cpp регистрирует слушателя InstanceTracker перед
# RUN_ALL_TESTS() - это и пишет Tracker.txt после каждого теста.
target_link_libraries(run_tests GTest::gtest)

# Абсолютный путь к папке tests/, чтобы Tracker.txt всегда писался туда,
# а не в текущую рабочую директорию процесса (которая зависит от того,
# из какой папки сборки/конфигурации запуска реально стартовал
# run_tests.exe - cmake-build-debug, build-san и т.п.). См.
# tests/instance_tracker.cpp::default_tracker_path().
#
# Путь передаётся через configure_file() (генерируем заголовок с #define),
# а НЕ через target_compile_definitions/-D на командной строке компилятора —
# так безопаснее, если путь к проекту содержит не-ASCII символы (например,
# кириллицу в имени пользователя Windows), см. пояснение в
# tests/tracker_config.h.in.
configure_file(
        ${CMAKE_CURRENT_SOURCE_DIR}/tests/tracker_config.h.in
        ${CMAKE_CURRENT_BINARY_DIR}/generated/tracker_config.h
        @ONLY
)
target_include_directories(run_tests PRIVATE ${CMAKE_CURRENT_BINARY_DIR}/generated)

# MSVC без этого флага угадывает кодировку исходников по локали системы
# (для русской Windows — обычно codepage 1251), а не считает их UTF-8.
# Сгенерированный tracker_config.h содержит путь с кириллицей без BOM —
# без /utf-8 MSVC мог бы неверно раскодировать эти байты в строковый
# литерал ещё на этапе компиляции. GCC/Clang по умолчанию и так считают
# исходники UTF-8, им это не нужно.

if(CMAKE_CXX_COMPILER_ID MATCHES "GNU")
    add_compile_options(-fsanitize=address -fno-omit-frame-pointer -g)
    add_link_options(-fsanitize=address)
endif()

if (MSVC)
    target_compile_options(run_tests PRIVATE /utf-8)
endif()

if (ENABLE_SANITIZERS)
    if (MSVC)
        target_compile_options(run_tests PRIVATE /fsanitize=address)
    else()
        target_compile_options(run_tests PRIVATE -fsanitize=address,undefined -fno-omit-frame-pointer -g)
        target_link_options(run_tests PRIVATE -fsanitize=address,undefined)
    endif()
endif()

# 6. Интеграция с интерфейсом тестирования CLion
include(GoogleTest)
gtest_discover_tests(run_tests)