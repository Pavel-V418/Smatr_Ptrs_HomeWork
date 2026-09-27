#include <gtest/gtest.h>
#include <stdexcept>
#include "../src/MsPtr.h"

// Создает сырой массив и границы для независимого тестирования MsPtr
class MsPtrTest : public ::testing::Test {
protected:
    int arr[5] = {10, 20, 30, 40, 50};
    int* begin = arr;
    int* end = arr + 5; // Указатель past-the-end
};

// 1. ИНИЦИАЛИЗАЦИЯ И ПОЛУИНТЕРВАЛ

TEST_F(MsPtrTest, InitializationAndBounds) {
    // Валидная инициализация в пределах массива
    EXPECT_NO_THROW(MsPtr<int> ptr(begin, begin, end));
    
    // Валидная инициализация указателем на конец (past-the-end)
    EXPECT_NO_THROW(MsPtr<int> ptr(end, begin, end));

    // Выброс исключений при физическом выходе за границы памяти
    EXPECT_THROW(MsPtr<int> ptr(begin - 1, begin, end), std::out_of_range);
    EXPECT_THROW(MsPtr<int> ptr(end + 1, begin, end), std::out_of_range);
}

// 2. БЕЗОПАСНОЕ РАЗЫМЕНОВАНИЕ

TEST_F(MsPtrTest, SafeDereference) {
    MsPtr<int> ptr(begin, begin, end);
    EXPECT_EQ(*ptr, 10);

    // Разыменование указателя past-the-end строго запрещено
    MsPtr<int> end_ptr(end, begin, end);
    EXPECT_THROW(*end_ptr, std::out_of_range);
}

// 3. ПРЕФИКСНЫЙ И ПОСТФИКСНЫЙ ИНКРЕМЕНТ

TEST_F(MsPtrTest, IncrementOperators) {
    MsPtr<int> ptr(begin, begin, end);
    
    // Проверка префиксного инкремента
    ++ptr;
    EXPECT_EQ(*ptr, 20);
    
    // Проверка постфиксного инкремента (возвращает старое значение)
    auto old_ptr = ptr++;
    EXPECT_EQ(*old_ptr, 20);
    EXPECT_EQ(*ptr, 30);

    // Сдвиг до конца и попытка выйти за пределы массива
    ++ptr; // 40
    ++ptr; // 50
    ++ptr; // end
    EXPECT_THROW(++ptr, std::out_of_range);
}

// 4. ПРЕФИКСНЫЙ И ПОСТФИКСНЫЙ ДЕКРЕМЕНТ (Decrement Operators)

TEST_F(MsPtrTest, DecrementOperators) {
    MsPtr<int> ptr(begin + 2, begin, end); // Указывает на 30
    
    // Проверка префиксного декремента
    --ptr;
    EXPECT_EQ(*ptr, 20);
    
    // Проверка постфиксного декремента
    auto old_ptr = ptr--;
    EXPECT_EQ(*old_ptr, 20);
    EXPECT_EQ(*ptr, 10);

    // Попытка выйти за нижнюю границу (левее begin)[cite: 1]
    EXPECT_THROW(--ptr, std::out_of_range);
}

// 5. АРИФМЕТИКА СМЕЩЕНИЙ

TEST_F(MsPtrTest, ArithmeticOffsets) {
    MsPtr<int> ptr(begin, begin, end);

    // Проверка сложения
    auto plus_ptr = ptr + 3;
    EXPECT_EQ(*plus_ptr, 40);

    // Проверка вычитания
    auto minus_ptr = plus_ptr - 2;
    EXPECT_EQ(*minus_ptr, 20);

    // Проверка блокировки при некорректном смещении[cite: 1]
    EXPECT_THROW(ptr + 6, std::out_of_range); // Правее end
    EXPECT_THROW(ptr - 1, std::out_of_range); // Левее begin
}

// 6. ОПЕРАТОРЫ СРАВНЕНИЯ ДЛЯ ЦИКЛОВ

TEST_F(MsPtrTest, ComparisonOperators) {
    MsPtr<int> ptr1(begin, begin, end);
    MsPtr<int> ptr2(begin, begin, end);
    MsPtr<int> ptr3(begin + 1, begin, end);

    // Проверка равенства и неравенства адресов[cite: 1]
    EXPECT_TRUE(ptr1 == ptr2);
    EXPECT_TRUE(ptr1 != ptr3);
}