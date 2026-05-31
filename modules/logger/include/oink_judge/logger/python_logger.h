#pragma once
#include "oink_judge/logger/logger.h"

#include <source_location>
#include <string>

namespace oink_judge::logger {

/// Forwards C++ log calls to Python's `logging` module.
/// The module name becomes the Python logger name and LogType maps to the Python log level.
/// This makes C++ logs go through the same Python logging pipeline as native Python logs.
class PythonLogger {
  public:
    /// Log a message via Python's `logging` module.
    /// The message is passed as-is — Python's logging formatters handle the output format.
    static auto log(const std::string& module, const std::string& message, LogType type, uint32_t level,
                    std::source_location location) -> void;
    // TODO add location support (currently ignored)
};

} // namespace oink_judge::logger
