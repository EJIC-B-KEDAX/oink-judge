#include "oink_judge/config/logger_utils.h"

#include "oink_judge/config/config.h"

#include <oink_judge/logger/logger.h>

namespace oink_judge::config {

auto getLoggerOutputStream() -> std::optional<std::string> {
    const auto& config_data = Config::config();

    if (!checkObjectIsObject(config_data, {"logger"})) {
        return std::nullopt;
    }

    if (!checkObjectIsString(config_data, {"logger", "output_stream"})) {
        return std::string("stderr");
    }
    return config_data["logger"]["output_stream"].get<std::string>();
}

auto getAllLoggerLogLevels() -> std::optional<std::map<std::string, uint32_t>> {
    const auto& config_data = Config::config();
    if (!checkObjectIsObject(config_data, {"logger"})) {
        return std::nullopt;
    }
    const auto& logger = config_data["logger"];
    if (!checkObjectIsObject(logger, {"log_levels"})) {
        return std::nullopt;
    }
    std::map<std::string, uint32_t> levels;
    for (auto it = logger["log_levels"].begin(); it != logger["log_levels"].end(); ++it) {
        if (it.value().is_number_unsigned()) {
            levels[it.key()] = it.value().get<uint32_t>();
        }
    }
    return levels;
}

auto getLoggerColorMap() -> std::optional<std::map<std::string, std::string>> {
    const auto& config_data = Config::config();
    if (!checkObjectIsObject(config_data, {"logger"})) {
        return std::nullopt;
    }
    const auto& logger = config_data["logger"];
    if (!checkObjectIsObject(logger, {"color_map"})) {
        return std::nullopt;
    }
    std::map<std::string, std::string> cmap;
    for (auto it = logger["color_map"].begin(); it != logger["color_map"].end(); ++it) {
        if (it.value().is_string()) {
            cmap[it.key()] = it.value().get<std::string>();
        }
    }
    return cmap;
}

auto getLoggerMinLocationLength() -> std::optional<uint32_t> {
    const auto& config_data = Config::config();
    if (!checkObjectIsObject(config_data, {"logger"})) {
        return std::nullopt;
    }
    const auto& logger = config_data["logger"];
    if (checkObjectIsNumberUnsigned(logger, {"min_location_length"})) {
        return logger["min_location_length"].get<uint32_t>();
    }
    return std::nullopt;
}

auto getLoggerMinModuleLength() -> std::optional<uint32_t> {
    const auto& config_data = Config::config();
    if (!checkObjectIsObject(config_data, {"logger"})) {
        return std::nullopt;
    }
    const auto& logger = config_data["logger"];
    if (checkObjectIsNumberUnsigned(logger, {"min_module_length"})) {
        return logger["min_module_length"].get<uint32_t>();
    }
    return std::nullopt;
}

auto getLoggerLogLevel(const std::string& module) -> std::optional<uint32_t> {
    auto levels_opt = getAllLoggerLogLevels();
    if (!levels_opt) {
        return std::nullopt;
    }
    const auto& levels = *levels_opt;
    auto iter = levels.find(module);
    if (iter != levels.end()) {
        return iter->second;
    }
    return std::nullopt;
}

auto getLoggerConfig() -> std::optional<LoggerConfig> {
    auto out = getLoggerOutputStream();
    auto levels = getAllLoggerLogLevels();
    auto cmap = getLoggerColorMap();
    auto min_loc = getLoggerMinLocationLength();
    auto min_mod = getLoggerMinModuleLength();

    // If none of the logger settings exist, return nullopt
    if (!out || !levels) {
        return std::nullopt;
    }

    LoggerConfig result;
    result.output_stream = out.value();
    result.log_levels = *levels;
    if (cmap) {
        result.color_map = cmap;
    }
    if (min_loc) {
        result.min_location_length = min_loc;
    }
    if (min_mod) {
        result.min_module_length = min_mod;
    }

    return result;
}

auto configureLogger(const LoggerConfig& config) -> void {
    // TODO set output stream here

    for (const auto& [module, level] : config.log_levels) {
        logger::Logger::instance().setLogLevel(module, level);
    }

    if (config.min_module_length.has_value()) {
        logger::Logger::instance().setMinModuleLength(config.min_module_length.value());
    }

    if (config.min_location_length.has_value()) {
        logger::Logger::instance().setMinLocationLength(config.min_location_length.value());
    }

    if (config.color_map.has_value()) {
        logger::Logger::instance().setColorMap(config.color_map.value());
    }
}

} // namespace oink_judge::config