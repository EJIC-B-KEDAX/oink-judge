#include "oink_judge/logger/logger.h"

using namespace oink_judge::logger;

auto foo(int param1, const std::string& param2) -> void {
    // Just a dummy function to test logging from different functions
    logMessage("TestModule", 1, "Logging from foo function.", LogType::INFO);
}

template <typename T> auto bar(T value) -> void {
    logMessage("TestModule", 1, "Logging from bar function.", LogType::INFO);
    logMessage("TestModule", 1, "Logging from bar function.", LogType::INFO, 1);
}

int main() {
    Logger::instance().setLogLevel("TestModule", 1);

    logMessage("TestModule", 1, "This is a debug message.", LogType::DEBUG);
    logMessage("TestModule", 1, "This is an info message.", LogType::INFO);
    logMessage("TestModule", 1, "This is a success message.", LogType::SUCCESS);
    logMessage("TestModule", 1, "This is a warning message.", LogType::WARNING);
    logMessage("TestModule", 1, "This is an error message.", LogType::ERROR);
    logMessage("TestModule", 1, "This is a critical message.", LogType::CRITICAL);

    logMessage("TestModule", 2, "This is unlogged message.", LogType::INFO);

    foo(42, "example"); // NOLINT
    bar(3.14);          // NOLINT
    return 0;
}