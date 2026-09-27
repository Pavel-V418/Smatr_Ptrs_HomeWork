#include <gtest/gtest.h>
#include <utility>
#include "../src/ShrdPtr.h"

// --- Вспомогательный класс-шпион для отслеживания утечек ---
struct InstanceTracker {
    inline static int active_instances = 0;
    int value;

    InstanceTracker(int v = 0) : value(v) { ++active_instances; }
    InstanceTracker(const InstanceTracker& other) : value(other.value) { ++active_instances; }
    virtual ~InstanceTracker() { --active_instances; }
};

struct DerivedTracker : public InstanceTracker {
    DerivedTracker(int v = 0) : InstanceTracker(v) {}
};

// 1. КОНТРОЛЬ СЧЕТЧИКА ССЫЛОК И ЖИЗНЕННОГО ЦИКЛА

TEST(ShrdPtrTest, SharedOwnershipAndDestruction) {
    InstanceTracker::active_instances = 0;
    {
        ShrdPtr<InstanceTracker> p1(new InstanceTracker(100));
        EXPECT_EQ(InstanceTracker::active_instances, 1);
        {
            ShrdPtr<InstanceTracker> p2 = p1; // Копирование, счетчик = 2
            EXPECT_EQ(InstanceTracker::active_instances, 1); // Объект в памяти по-прежнему один
            EXPECT_EQ(p2->value, 100);
        }
        // p2 уничтожен, счетчик = 1. Объект должен жить.
        EXPECT_EQ(InstanceTracker::active_instances, 1);
    }
    // p1 уничтожен, счетчик = 0. Объект должен быть удален.
    EXPECT_EQ(InstanceTracker::active_instances, 0);
}

TEST(ShrdPtrTest, ResetDropsReferenceCorrectly) {
    InstanceTracker::active_instances = 0;
    ShrdPtr<InstanceTracker> p1(new InstanceTracker(1));
    ShrdPtr<InstanceTracker> p2 = p1;

    EXPECT_EQ(InstanceTracker::active_instances, 1);

    // p1 отказывается от старого объекта и берет новый
    p1.reset(new InstanceTracker(2));

    // Теперь в памяти 2 независимых объекта
    EXPECT_EQ(InstanceTracker::active_instances, 2);
    EXPECT_EQ(p1->value, 2);
    EXPECT_EQ(p2->value, 1);
}

// 2. COPY- И MOVE-СЕМАНТИКА

TEST(ShrdPtrTest, CopyAssignmentOverwritesAndCleans) {
    InstanceTracker::active_instances = 0;
    ShrdPtr<InstanceTracker> p1(new InstanceTracker(1));
    {
        ShrdPtr<InstanceTracker> p2(new InstanceTracker(2));
        EXPECT_EQ(InstanceTracker::active_instances, 2);

        // p2 отписывается от объекта "2" (счетчик 0 -> удаление) и подписывается на "1"
        p2 = p1;
        EXPECT_EQ(InstanceTracker::active_instances, 1);
        EXPECT_EQ(p2->value, 1);

        // Самоприсваивание не должно ничего сломать
        p2 = p2;
        EXPECT_EQ(InstanceTracker::active_instances, 1);
    }
    // p2 умер, но p1 все еще держит объект "1"
    EXPECT_EQ(InstanceTracker::active_instances, 1);
}

TEST(ShrdPtrTest, MoveSemanticsTransfersOwnershipWithoutIncrement) {
    InstanceTracker::active_instances = 0;
    ShrdPtr<InstanceTracker> p1(new InstanceTracker(42));

    // Передаем владение: счетчик не должен расти, объект не должен копироваться
    ShrdPtr<InstanceTracker> p2(std::move(p1));
    EXPECT_EQ(InstanceTracker::active_instances, 1);
    EXPECT_EQ(p2->value, 42);

    ShrdPtr<InstanceTracker> p3(new InstanceTracker(99));
    // Move-присваивание: p3 удаляет свой "99" и забирает "42" у p2
    p3 = std::move(p2);
    EXPECT_EQ(InstanceTracker::active_instances, 1);
    EXPECT_EQ(p3->value, 42);
}

// 3. БЕЗОПАСНАЯ РАБОТА С МАССИВАМИ

TEST(ShrdPtrTest, ArraySpecializationHandlesDeleteArray) {
    InstanceTracker::active_instances = 0;
    {
        // Создаем массив из 3 элементов.
        ShrdPtr<InstanceTracker[]> arr_ptr(new InstanceTracker[3]);
        EXPECT_EQ(InstanceTracker::active_instances, 3);

        {
            ShrdPtr<InstanceTracker[]> arr_copy = arr_ptr;
            arr_copy[0].value = 77;
            EXPECT_EQ(arr_ptr[0].value, 77); // Проверка разделяемого доступа
        }
        // arr_copy умер, но arr_ptr держит массив
        EXPECT_EQ(InstanceTracker::active_instances, 3);
    }
    // Здесь сработает clean(), который через if constexpr вызовет delete[]
    EXPECT_EQ(InstanceTracker::active_instances, 0);
}

// 4. ПОДТИПИЗАЦИЯ И ПОЛИМОРФИЗМ

TEST(ShrdPtrTest, SubtypingCopyAndMove) {
    InstanceTracker::active_instances = 0;
    {
        ShrdPtr<DerivedTracker> derived(new DerivedTracker(88));

        // Базовый класс разделяет владение с наследником через шаблонный конструктор копирования
        ShrdPtr<InstanceTracker> base_copy(derived);
        EXPECT_EQ(InstanceTracker::active_instances, 1);
        EXPECT_EQ(base_copy->value, 88);

        // Базовый класс забирает владение у наследника через шаблонный move-конструктор
        ShrdPtr<InstanceTracker> base_moved(std::move(derived));
        EXPECT_EQ(InstanceTracker::active_instances, 1);
    }
    // При разрушении базовых указателей виртуальный деструктор корректно очистит DerivedTracker
    EXPECT_EQ(InstanceTracker::active_instances, 0);
}