#pragma once

#include "UnqPtr.h"
#include "ShrdPtr.h"
#include "MsPtr.h"
#include <stdexcept>

template <class T>
class MemorySpan {

private:
    UnqPtr<T[]> data;
    int size;
    int capacity;

    void check_index(int index) {
        if (index < 0 || index >= size)
            throw std::out_of_range("Index out of range");
    }


public:
    explicit MemorySpan(int init_size = 0){
        if (init_size < 0)
            throw std::invalid_argument("Size cannot be negative");

        size = init_size;
        capacity = init_size;
        if (init_size > 0)
            data.reset(new T[init_size]);
    }

    MemorySpan(const MemorySpan&) = delete;
    MemorySpan& operator=(const MemorySpan&) = delete;

    // Move-семантика
    MemorySpan(MemorySpan&& other) noexcept
        : data(static_cast<UnqPtr<T[]>&&>(other.data)), size(other.size), capacity(other.capacity) {
        other.size = 0;
        other.capacity = 0;
    }

    MemorySpan& operator= (MemorySpan&& other) noexcept {
        if (this != &other) {
            data = static_cast<UnqPtr<T[]>&&>(other.data);
            size = other.size;
            capacity = other.capacity;

            other.size = 0;
            other.capacity = 0;
        }

        return *this;
    }

    int get_size() const {
        return size;
    }

    int get_capacity() const {
        return capacity;
    }

    void set(int index, const T& value) {
        check_index(index);

        data[index] = value;
    }

    UnqPtr<T> get(int index) {
        check_index(index);

        return UnqPtr<T>(new T(data[index]));
    }

    ShrdPtr<T> copy(int index) {
        check_index(index);

        return ShrdPtr<T>(new T(data[index]));
    }

    MsPtr<T> locate(int index) {
        check_index(index);

        T* base_ptr = data.get();  // Вытаскиваем базовый сырой указатель из UnqPtr

        return MsPtr<T>(base_ptr + index, base_ptr, base_ptr + size);
    }

};