#pragma once
#include "oink_judge/config/common_utils.h"

#include <oink_judge/logger/logger.h>

#include <stdexcept>

namespace oink_judge::config {

template <typename T>
auto requireHasValue(const std::optional<T>& opt, const std::string& message, const std::string& module, int level,
                     std::source_location location) -> const T& {
    if (!opt.has_value()) {
        logger::logError(module, message, level, location);
        throw std::runtime_error(message);
    }
    return *opt;
}

} // namespace oink_judge::config
