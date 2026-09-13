cmake_minimum_required(VERSION 4.2)
project(HM_SmatrPtrs)

set(CMAKE_CXX_STANDARD 20)

add_executable(HM_SmatrPtrs main.cpp
        src/UnqPtr.h
        src/ShrdPtr.h
        src/MemorySpan.h
        src/MsPtr.h)
