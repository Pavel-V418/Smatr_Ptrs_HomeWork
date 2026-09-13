#pragma once

#include <stdexcept>

template <class T>
class MsPtr {

private:
    T* current;
    T* begin;
    T* end;

public:
    MsPtr(T* current, T* begin, T* end)
        : current(current), begin(begin), end(end) {
        if (current < begin || current > end)
            throw std::out_of_range("Pointer out of range");
    }

    MsPtr& operator++() {
        if (current >= end)
            throw std::out_of_range("MsPtr out of upper bound");

        ++current;
        return *this;
    }

    MsPtr operator++(int) { // int в скобках - костыль, помогающий компилятору определить перегрузку
        MsPtr temp = *this;
        ++(*this);
        return temp;
    }

    MsPtr& operator--() {
        if (current <= begin)
            throw std::out_of_range("MsPtr out of lower bound");

        --current;
        return *this;
    }

    MsPtr operator--(int) {
        MsPtr temp = *this;
        --(*this);
        return temp;
    }

    MsPtr operator+(int offset) const{
        if (offset < 0 || current + offset > end)
            throw std::out_of_range("MsPtr out of range");
        return MsPtr(current + offset, begin, end);
    }

    MsPtr operator-(int offset) const{
        if (offset < 0 || current - offset < begin)
            throw std::out_of_range("MsPtr out of range");
        return MsPtr(current - offset, begin, end);
    }

    T& operator*() const {
        if (current >= end)
            throw std::out_of_range("Cannot dereference end pointer");
        return *current;
    }
    T* operator->() const {
        if (current >= end)
            throw std::out_of_range("Cannot dereference end pointer");
        return current;
    }
};