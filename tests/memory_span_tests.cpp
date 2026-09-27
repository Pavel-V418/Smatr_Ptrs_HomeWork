#include <gtest/gtest.h>
#include <utility>
#include "../src/MemorySpan.h"
#include "../src/MsPtr.h"
#include "../src/UnqPtr.h"
#include "../src/ShrdPtr.h"

// --- Вспомогательный класс-шпион для отслеживания утечек ---
struct InstanceTracker {
    inline static int active_instances = 0;
    int value;

    InstanceTracker(int v = 0) : value(v) { ++active_instances; }
    InstanceTracker(const InstanceTracker& other) : value(other.value) { ++active_instances; }
    virtual ~InstanceTracker() { --active_instances; }
};

// 1. ИНИЦИАЛИЗАЦИЯ И КОНТРОЛЬ ГРАНИЦ

TEST(MemorySpanTest, CapacityAndOutOfBounds) {
    MemorySpan<int> span(5);

    EXPECT_EQ(span.get_size(), 5);
    EXPECT_EQ(span.get_capacity(), 5);

    // Проверка выброса исключений при выходе за границы
    EXPECT_THROW(span.locate(5), std::out_of_range);
    EXPECT_THROW(span.get(5), std::out_of_range);
    EXPECT_THROW(span.copy(5), std::out_of_range);

}

// 2. БЕЗОПАСНАЯ НАВИГАЦИЯ И ЗАПИСЬ (Locate & MsPtr Arithmetic)

TEST(MemorySpanTest, LocateProvidesSafeAccess) {
    MemorySpan<int> span(3);

    // Инициализация внутреннего массива через безопасный указатель MsPtr
    for (size_t i = 0; i < span.get_size(); ++i) {
        *span.locate(i) = (i + 1) * 10; // Запишет 10, 20, 30
    }

    EXPECT_EQ(*span.locate(0), 10);
    EXPECT_EQ(*span.locate(2), 30);

    // Проверка адресной арифметики выданного MsPtr
    auto ptr = span.locate(0);
    EXPECT_EQ(*(ptr + 1), 20);

    ++ptr; // Сдвиг на 1 элемент
    EXPECT_EQ(*ptr, 20);

    // Попытка выйти за границы массива через арифметику MsPtr
    EXPECT_THROW(ptr + 5, std::out_of_range);
}

// 3. ИЗОЛЯЦИЯ ВЫДАВАЕМЫХ КОПИЙ (Get & Copy Mechanics)

TEST(MemorySpanTest, GetAndCopyReturnIndependentInstances) {
    MemorySpan<int> span(2);
    *span.locate(0) = 42;
    *span.locate(1) = 99;

    // Проверка эксклюзивной копии через Get()
    UnqPtr<int> unq = span.get(0);
    EXPECT_EQ(*unq, 42);
    *unq = 100; // Изменяем выданную копию
    EXPECT_EQ(*span.locate(0), 42); // Оригинал в контейнере должен остаться неизменным

    // Проверка разделяемой копии через Copy()
    ShrdPtr<int> shrd = span.copy(1);
    EXPECT_EQ(*shrd, 99);
    *shrd = 200; // Изменяем выданную копию
    EXPECT_EQ(*span.locate(1), 99); // Оригинал в контейнере должен остаться неизменным
}

// 4. MOVE-СЕМАНТИКА КОНТЕЙНЕРА (MoveSemantics)

TEST(MemorySpanTest, MoveConstructorAndAssignmentTransferBuffer) {
    MemorySpan<int> span1(3);
    *span1.locate(0) = 77;

    // Передаем владение буфером новому контейнеру
    MemorySpan<int> span2(std::move(span1));
    EXPECT_EQ(span1.get_size(), 0);
    EXPECT_EQ(span1.get_capacity(), 0);
    EXPECT_EQ(span2.get_size(), 3);
    EXPECT_EQ(*span2.locate(0), 77);

    MemorySpan<int> span3;
    // Передаем владение через оператор присваивания
    span3 = std::move(span2);
    EXPECT_EQ(span2.get_size(), 0);
    EXPECT_EQ(span3.get_size(), 3);
    EXPECT_EQ(*span3.locate(0), 77);
}

// 5. КОНТРОЛЬ ПАМЯТИ И УТЕЧЕК

TEST(MemorySpanTest, MemorySpanCleansUpCorrectly) {
    InstanceTracker::active_instances = 0;
    {
        // Создаем массив на 4 объекта
        MemorySpan<InstanceTracker> span(4);
        EXPECT_EQ(InstanceTracker::active_instances, 4);

        {
            // Метод Copy() создает в куче 1 новый независимый объект
            ShrdPtr<InstanceTracker> shrd = span.copy(0);
            EXPECT_EQ(InstanceTracker::active_instances, 5);
        }
        // shrd умер, его копия удалена, счетчик вернулся к 4
        EXPECT_EQ(InstanceTracker::active_instances, 4);

        {
            // Метод Get() создает в куче 1 новый независимый объект
            UnqPtr<InstanceTracker> unq = span.get(1);
            EXPECT_EQ(InstanceTracker::active_instances, 5);
        }
        // unq умер, его копия удалена, счетчик вернулся к 4
        EXPECT_EQ(InstanceTracker::active_instances, 4);
    }
    // Контейнер MemorySpan вышел из области видимости, встроенный UnqPtr<T[]> вызвал delete[]
    EXPECT_EQ(InstanceTracker::active_instances, 0);
}

// тест для сета