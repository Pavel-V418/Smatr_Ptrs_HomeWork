#pragma once

#include <type_traits>

template <class T>
class UnqPtr {

using Type = std::remove_extent_t<T>;

private:
    Type* ptr;

    void clean(Type* p) {
        if constexpr (std::is_array<T>::value)
            delete[] p;
        else
            delete p;
    }

    template <class U>
    friend class UnqPtr;

public:
    explicit UnqPtr(Type* ptr = nullptr)
        : ptr(ptr) {}

    ~UnqPtr() {
        clean(ptr);
    }

    UnqPtr(const UnqPtr&) = delete; // запрет конструктора копирования
    UnqPtr& operator= (const UnqPtr&) = delete; // запрет присваивания копированием

    // move-семантика
    UnqPtr(UnqPtr&& other) noexcept
        : ptr(other.ptr) {
        other.ptr = nullptr;
    }

    UnqPtr& operator= (UnqPtr&& other) noexcept {
        if (this != &other) {
            clean(ptr);

            ptr = other.ptr;
            other.ptr = nullptr;
        }
        return *this;
    }

    // SubTyping
    template <class U>
    UnqPtr(const UnqPtr<U>& other) = delete;

    template <class U>
    UnqPtr& operator= (UnqPtr& other) = delete;

    template <class U>
    UnqPtr(UnqPtr<U>&& other) noexcept
        : ptr(static_cast<Type*>(other.ptr)) {
        other.ptr = nullptr;
    }

    template <class U>
    UnqPtr& operator= (UnqPtr<U>&& other) noexcept {
        clean(ptr);

        ptr = static_cast<Type*>(other.ptr);
        other.ptr = nullptr;
        return *this;
    }

    Type& operator* () const {return *ptr;}
    Type* operator-> () const {return ptr;}
    Type& operator[] (int index) const {return ptr[index];}

    Type* get() const{return ptr;}

    Type* release() {
        Type* tmp = ptr;
        ptr = nullptr;
        return tmp;
    }

    void reset(Type* p = nullptr) {
        if (ptr != p) {
            clean(ptr);
            ptr = p;
        }
    }

};


