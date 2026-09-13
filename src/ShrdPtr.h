#pragma once

#include <type_traits>

template <class T>
class ShrdPtr {

using Type = std::remove_extent_t<T>;

private:
    Type* ptr;
    int* ptrs_counter;

    template <class U>
    friend class ShrdPtr;

    void clean(Type* p) {
        if constexpr (std::is_array<T>::value)
            delete[] p;
        else
            delete p;
    }

    void counter_check_for_remove(Type* pointer,int* pointers_counter) {
        if (pointers_counter && --(*pointers_counter) == 0) {
            clean(pointer);
            delete pointers_counter;
        }
    }

public:
    explicit ShrdPtr(Type* p = nullptr)
        : ptr(p), ptrs_counter(new int(1)) {}

    ~ShrdPtr() {
        counter_check_for_remove(ptr, ptrs_counter);
    }

    // присваивание и копирование
    ShrdPtr(const ShrdPtr& other) {
        if (this != &other) {
            ptr = other.ptr;
            ptrs_counter = other.ptrs_counter;

            if (ptrs_counter)
                ++(*ptrs_counter);
        }
    }

    ShrdPtr& operator= (const ShrdPtr& other) {
        if (this != &other) {
            counter_check_for_remove(ptr, ptrs_counter);

            ptr = other.ptr;
            ptrs_counter = other.ptrs_counter;

            if (ptrs_counter)
                ++(*ptrs_counter);
        }
        return *this;
    }

    // move-семантика
    ShrdPtr(ShrdPtr&& other) noexcept
        : ptr(other.ptr), ptrs_counter(other.ptrs_counter) {
        other.ptr = nullptr;
        other.ptrs_counter = nullptr;
    }

    ShrdPtr& operator= (ShrdPtr&& other) noexcept {
        if (this != &other) {
            counter_check_for_remove(ptr, ptrs_counter);

            ptr = other.ptr;
            ptrs_counter = other.ptrs_counter;

            other.ptr = nullptr;
            other.ptrs_counter = nullptr;
        }
        return *this;
    }

    // Подтипизация
    template <class U>
    ShrdPtr(const ShrdPtr<U>& other)
        : ptr(static_cast<Type*>(other.ptr)), ptrs_counter(other.ptrs_counter) {
        if (ptrs_counter)
            ++(*ptrs_counter);
    }

    template <class U>
    ShrdPtr& operator= (const ShrdPtr<U>& other) {
        counter_check_for_remove(ptr, ptrs_counter);

        ptr = static_cast<Type*>(other.ptr);
        ptrs_counter = other.ptrs_counter;

        if (ptrs_counter)
            ++(*ptrs_counter);

        return *this;
    }

    template <class U>
    ShrdPtr(ShrdPtr<U>&& other) noexcept
        : ptr(static_cast<Type*>(other.ptr)), ptrs_counter(other.ptrs_counter) {
        other.ptr = nullptr;
        other.ptrs_counter = nullptr;
    }

    template <class U>
    ShrdPtr& operator= (ShrdPtr<U>&& other) noexcept {
        counter_check_for_remove(ptr, ptrs_counter);

        ptr = static_cast<Type*>(other.ptr);
        ptrs_counter = other.ptrs_counter;

        other.ptr = nullptr;
        other.ptrs_counter = nullptr;

        return *this;
    }

    Type& operator*() const {return *ptr;}
    Type* operator->() const {return ptr;}
    Type& operator[](int index) const {return ptr[index];}

    void reset(Type* p = nullptr) {
        if (ptr != p) {
            counter_check_for_remove(ptr, ptrs_counter);

            ptr = p;
            ptrs_counter = new int(1);
        }
    }

};