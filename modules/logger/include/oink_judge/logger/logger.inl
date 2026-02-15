#pragma once
#include "oink_judge/logger/logger.h"

#include <stdexcept>

namespace oink_judge::logger {

template <typename T>
auto requireHasValue(const std::optional<T>& opt, const std::string& message, const std::string& module, int level,
                     uint32_t full_function_name_on_level, std::source_location location) -> const T& {
    if (!opt.has_value()) {
        logError(module, message, level, full_function_name_on_level, location);
        throw std::runtime_error(message);
    }
    return *opt;
}

} // namespace oink_judge::logger
