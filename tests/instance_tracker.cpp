#include "instance_tracker.h"

#include <filesystem>
#include <fstream>

// Сгенерирован CMake из tests/tracker_config.h.in (см. configure_file()
// в CMakeLists.txt) - определяет TRACKER_OUTPUT_DIR как абсолютный путь
// к папке tests/ в исходниках проекта.
#include "tracker_config.h"

std::string default_tracker_path() {
#ifdef TRACKER_OUTPUT_DIR
    return std::string(TRACKER_OUTPUT_DIR) + "/Tracker.txt";
#else
    return "Tracker.txt";
#endif
}

namespace {

// Фикс проблемы несоздания файла Tracker.txt в папке tests, связанной с кирилицей
std::filesystem::path utf8_to_path(const std::string& utf8_text) {
    const auto* begin = reinterpret_cast<const char8_t*>(utf8_text.data());
    const auto* end = begin + utf8_text.size();
    return std::filesystem::path(begin, end);
}

}  // namespace

void save_tracker_to_file(const std::string& test_name, const std::string& filename) {
    const std::filesystem::path path = utf8_to_path(filename);
    const bool file_already_exists = std::filesystem::exists(path);

    // ios::app создаёт файл, если его нет, и всегда пишет в конец,
    // если он уже есть — то есть дозапись получаем "бесплатно".
    std::ofstream out(path, std::ios::app);
    if (!out.is_open())
        return; // не валим тест из-за проблем с файловой системой

    if (!file_already_exists) {
        out << "=== Отчёт InstanceTracker (учёт создания/удаления объектов) ===\n";
        out << "Формат строки: [тест] создано=N удалено=M активно_сейчас(утечка)=K -> статус\n\n";
    }

    const long long created = InstanceTracker::total_created;
    const long long destroyed = InstanceTracker::total_destroyed;
    const int leaked = InstanceTracker::active_instances;

    out << "[" << test_name << "] "
        << "создано=" << created << " "
        << "удалено=" << destroyed << " "
        << "активно_сейчас(утечка)=" << leaked
        << (leaked == 0 ? " -> OK, утечек нет" : " -> ВНИМАНИЕ: обнаружена утечка!")
        << "\n";
}

namespace {

// Слушатель GTest: обнуляет InstanceTracker перед каждым тестом и
// сохраняет отчёт в файл после его завершения. К моменту OnTestEnd тело
// теста уже полностью отработало вместе со всеми деструкторами локальных
// объектов (это гарантирует сам GTest), поэтому в отличие от ручного
// вызова save_tracker_to_file() в конце тела теста здесь не будет ложных
// "утечек" из-за объектов, которые ещё физически живы, но корректно
// умрут при выходе из области видимости на следующей же строке.
class InstanceTrackerListener : public ::testing::EmptyTestEventListener {
    void OnTestStart(const ::testing::TestInfo& /*test_info*/) override {
        InstanceTracker::reset();
    }

    void OnTestEnd(const ::testing::TestInfo& test_info) override {
        // Тест ни разу не создал InstanceTracker (например, MsPtrTest,
        // работающий с сырым int[]) - логировать нечего.
        if (InstanceTracker::total_created == 0)
            return;

        const std::string full_name =
            std::string(test_info.test_suite_name()) + "." + test_info.name();
        save_tracker_to_file(full_name);
    }
};

}  // namespace

void register_instance_tracker_listener() {
    ::testing::UnitTest::GetInstance()->listeners().Append(new InstanceTrackerListener());
}
