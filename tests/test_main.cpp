#include <gtest/gtest.h>
#include "instance_tracker.h"

// Свой main() вместо готового GTest::gtest_main нужен только для одной
// вещи: зарегистрировать слушателя InstanceTracker (см. instance_tracker.h)
// ДО RUN_ALL_TESTS(), чтобы он видел вообще все тесты.
int main(int argc, char** argv) {
    ::testing::InitGoogleTest(&argc, argv);
    register_instance_tracker_listener();
    return RUN_ALL_TESTS();
}
