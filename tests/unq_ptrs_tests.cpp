#include <gtest/gtest.h>
#include <utility>
#include "../src/UnqPtr.h"

// Вспомогательный класс-шпион для отслеживания утечек
struct InstanceTracker {
    inline static int active_instances = 0;
    int value;

    InstanceTracker(int v = 0) : value(v) {
        ++active_instances;
    }
    
    InstanceTracker(const InstanceTracker& other) : value(other.value) {
        ++active_instances;
    }
    
    // Виртуальный деструктор обязателен для корректной работы полиморфизма
    virtual ~InstanceTracker() {
        --active_instances;
    }
};

// Класс-наследник для проверки подтипизации (Subtyping)
struct DerivedTracker : public InstanceTracker {
    DerivedTracker(int v = 0) : InstanceTracker(v) {}
};

// 1. КОНТРОЛЬ ЖИЗНЕННОГО ЦИКЛА

TEST(UnqPtrTest, ScopeDestruction) {
    InstanceTracker::active_instances = 0;
    {
        UnqPtr<InstanceTracker> ptr(new InstanceTracker(42));
        EXPECT_EQ(InstanceTracker::active_instances, 1);
        EXPECT_EQ(ptr->value, 42);
        EXPECT_EQ((*ptr).value, 42);
    }
    // При выходе из области видимости объект должен быть удален
    EXPECT_EQ(InstanceTracker::active_instances, 0);
}

TEST(UnqPtrTest, ResetDeletesOldObject) {
    InstanceTracker::active_instances = 0;
    UnqPtr<InstanceTracker> ptr(new InstanceTracker(1));
    EXPECT_EQ(InstanceTracker::active_instances, 1);
    
    // Перезапись новым объектом
    ptr.reset(new InstanceTracker(2));
    EXPECT_EQ(InstanceTracker::active_instances, 1); // Старый удален, новый создан
    EXPECT_EQ(ptr->value, 2);
    
    // Самоприсваивание (защита от самоудаления)
    InstanceTracker* raw = ptr.get();
    ptr.reset(raw);
    EXPECT_EQ(InstanceTracker::active_instances, 1); 
    EXPECT_EQ(ptr.get(), raw);
}

// 2. MOVE-СЕМАНТИКА И ИЗОЛЯЦИЯ

TEST(UnqPtrTest, MoveConstructorTransfersOwnership) {
    InstanceTracker::active_instances = 0;
    UnqPtr<InstanceTracker> p1(new InstanceTracker(10));
    
    // Передаем владение
    UnqPtr<InstanceTracker> p2(std::move(p1));
    
    EXPECT_EQ(p1.get(), nullptr);
    ASSERT_NE(p2.get(), nullptr);
    EXPECT_EQ(p2->value, 10);
    EXPECT_EQ(InstanceTracker::active_instances, 1); // Объект в памяти остался один
}

TEST(UnqPtrTest, MoveAssignmentOverwritesAndCleans) {
    InstanceTracker::active_instances = 0;
    UnqPtr<InstanceTracker> p1(new InstanceTracker(1));
    UnqPtr<InstanceTracker> p2(new InstanceTracker(2));
    
    EXPECT_EQ(InstanceTracker::active_instances, 2);
    
    // p2 должен удалить свой объект (2) и забрать объект у p1 (1)
    p2 = std::move(p1);
    
    EXPECT_EQ(InstanceTracker::active_instances, 1);
    EXPECT_EQ(p1.get(), nullptr);
    EXPECT_EQ(p2->value, 1);
}

// 3. БЕЗОПАСНАЯ РАБОТА С МАССИВАМИ

TEST(UnqPtrTest, ArraySpecializationHandlesDeleteArray) {
    InstanceTracker::active_instances = 0;
    {
        // Создаем массив из 5 элементов. 
        UnqPtr<InstanceTracker[]> arr_ptr(new InstanceTracker[5]);
        EXPECT_EQ(InstanceTracker::active_instances, 5);
        
        arr_ptr[0].value = 100;
        arr_ptr[4].value = 500;
        
        EXPECT_EQ(arr_ptr[0].value, 100);
        EXPECT_EQ(arr_ptr[4].value, 500);
    }
    // Если бы внутри вызвался `delete` вместо `delete[]`, счетчик не стал бы равен 0.
    EXPECT_EQ(InstanceTracker::active_instances, 0); 
}

// 4. ПОДТИПИЗАЦИЯ И ПОЛИМОРФИЗМ

TEST(UnqPtrTest, SubtypingPolymorphism) {
    InstanceTracker::active_instances = 0;
    {
        UnqPtr<DerivedTracker> derived(new DerivedTracker(99));
        
        // Базовый класс забирает владение у наследника
        UnqPtr<InstanceTracker> base(std::move(derived));
        
        EXPECT_EQ(derived.get(), nullptr);
        ASSERT_NE(base.get(), nullptr);
        EXPECT_EQ(base->value, 99);
        EXPECT_EQ(InstanceTracker::active_instances, 1);
    }
    EXPECT_EQ(InstanceTracker::active_instances, 0);
}

// 5. ОТКАЗ ОТ ВЛАДЕНИЯ

TEST(UnqPtrTest, ReleaseRelinquishesOwnership) {
    InstanceTracker::active_instances = 0;
    InstanceTracker* raw = nullptr;
    
    {
        UnqPtr<InstanceTracker> ptr(new InstanceTracker(77));
        raw = ptr.release();
        
        EXPECT_EQ(ptr.get(), nullptr);
        EXPECT_EQ(InstanceTracker::active_instances, 1); // Объект НЕ удален
    }
    
    // После выхода из блока объект все еще жив, так как UnqPtr от него отказался
    EXPECT_EQ(InstanceTracker::active_instances, 1); 
    
    // Удаляем вручную, чтобы ASan не ругался на утечку в самом тесте
    delete raw; 
    EXPECT_EQ(InstanceTracker::active_instances, 0);
}