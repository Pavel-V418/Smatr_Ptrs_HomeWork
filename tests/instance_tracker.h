#pragma once

#include <gtest/gtest.h>
#include <string>

struct InstanceTracker {
    inline static int active_instances = 0;
    inline static long long total_created = 0;
    inline static long long total_destroyed = 0;

    int value;

    explicit InstanceTracker(int v = 0) : value(v) {
        ++active_instances;
        ++total_created;
    }

    InstanceTracker(const InstanceTracker& other) : value(other.value) {
        ++active_instances;
        ++total_created;
    }
    
    virtual ~InstanceTracker() {
        --active_instances;
        ++total_destroyed;
    }

    static void reset() {
        active_instances = 0;
        total_created = 0;
        total_destroyed = 0;
    }
};

// Класс-наследник для проверки подтипизации в ShrdPtr/UnqPtr
struct DerivedTracker : public InstanceTracker {
    explicit DerivedTracker(int v = 0) : InstanceTracker(v) {}
};

// Путь к Tracker.txt по умолчанию
std::string default_tracker_path();

void save_tracker_to_file(const std::string& test_name, const std::string& filename = default_tracker_path());

void register_instance_tracker_listener();
