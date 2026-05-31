#include "oink_judge/logger/python_logger.h"

#include "oink_judge/logger/logger.h"

#include <pybind11/pybind11.h>

#include <source_location>

namespace oink_judge::logger {

namespace py = pybind11;

namespace {

auto logTypeToPythonLevel(LogType type) -> int {
    // Python logging levels: DEBUG=10, INFO=20, WARNING=30, ERROR=40, CRITICAL=50
    // SUCCESS has no Python equivalent — map it to INFO.
    switch (type) {
    case LogType::DEBUG:
        return 10; // NOLINT
    case LogType::INFO:
    case LogType::SUCCESS:
        return 20; // NOLINT
    case LogType::WARNING:
        return 30; // NOLINT
    case LogType::ERROR:
        return 40; // NOLINT
    case LogType::CRITICAL:
        return 50; // NOLINT
    }
    return 20; // NOLINT
}

} // namespace

auto PythonLogger::log(const std::string& module, const std::string& message, LogType type, uint32_t level,
                       std::source_location location) -> void {
    auto& logger = Logger::instance();
    if (!logger.isLoggingEnabled(module, level)) {
        return;
    }

    py::gil_scoped_acquire gil;
    py::module_ logging = py::module_::import("logging");
    py::object py_logger = logging.attr("getLogger")(module);
    py_logger.attr("log")(logTypeToPythonLevel(type), "%s", message);
}

} // namespace oink_judge::logger
