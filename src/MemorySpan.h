#pragma once

#include "UnqPtr.h"
#include "ShrdPtr.h"
#include <stdexcept>

template <class T>
class MemorySpan {

private:
    UnqPtr<T[]> data;
    int size;
    int capacity;


public:
    explicit MemorySpan(int init_size = 0){
        if (init_size < 0)
            throw std::invalid_argument("Size cannot be negative");

        size = init_size;
        capacity = init_size;
        if (init_size > 0)
            data.reset(new T[init_size]);
    }

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

    UnqPtr<T> get(int index) {
        if (index < 0 || index >= size)
            throw std::out_of_range("Index out of range");
        return UnqPtr<T>(new T(data[index]));
    }

    ShrdPtr<T> copy(int index) {
        if (index < 0 || index >= size)
            throw std::out_of_range("Index out of range");

        return ShrdPtr<T>(new T(data[index]));
    }


};