#pragma once

#include <type_traits>

template <class T>
class ShrdPtr {

using Type = std::remove_extent_t<T>;

private:
    Type* ptr;
    int* ptrs_counter;

    void clean(Type* p) {
        if constexpr (std::is_array<T>::value)
            delete[] p;
        else
            delete p;
    }

public:
    explicit ShrdPtr(Type* p = nullptr)
        : ptr(p) {
        if (p != nullptr)
            ptrs_counter = new int(1);
        else
            ptrs_counter = new int(0);
        }

    ~ShrdPtr() {
        if (--(*ptrs_counter) == 0) {
            clean(ptr);
            delete ptrs_counter;
        }
    }

    // присваивание и копирование
    ShrdPtr(const ShrdPtr& other) {
        if (this != &other) {
            ptr = other.ptr;
            ptrs_counter++;
        }
    }

    ShrdPtr& operator= (const ShrdPtr& other) {
        if (this != &other) {
            ptr = other.ptr;
            ptrs_counter == other.ptrs_counter;
            ptrs_counter++;
        }
        return *this;
    }

    // move-семантика
    

};