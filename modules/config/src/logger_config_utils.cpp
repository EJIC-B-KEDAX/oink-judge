#include "oink_judge/config/logger_config_utils.h"

#include "oink_judge/config/config.h"

namespace oink_judge::config {

auto getLoggerOutputStream() -> std::optional<std::string> {
    const auto& config_data = Config::config();
    if (!config_data.contains("logger") || !config_data["logger"].is_object()) {
        return std::nullopt;
    }
    const auto& logger = config_data["logger"];
    if (logger.contains("output_stream") && logger["output_stream"].is_string()) {
        return logger["output_stream"].get<std::string>();
    }
    return std::string("stderr");
}

auto getAllLoggerLogLevels() -> std::optional<std::map<std::string, uint32_t>> {
    const auto& config_data = Config::config();
    if (!config_data.contains("logger") || !config_data["logger"].is_object()) {
        return std::nullopt;
    }
    const auto& logger = config_data["logger"];
    if (!logger.contains("log_level") || !logger["log_level"].is_object()) {
        return std::nullopt;
    }
    std::map<std::string, uint32_t> levels;
    for (auto it = logger["log_level"].begin(); it != logger["log_level"].end(); ++it) {
        if (it.value().is_number_integer()) {
            levels[it.key()] = it.value().get<uint32_t>();
        }
    }
    return levels;
}

auto getLoggerColorMap() -> std::optional<std::map<std::string, std::string>> {
    const auto& config_data = Config::config();
    if (!config_data.contains("logger") || !config_data["logger"].is_object()) {
        return std::nullopt;
    }
    const auto& logger = config_data["logger"];
    if (!logger.contains("color_map") || !logger["color_map"].is_object()) {
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
    if (!config_data.contains("logger") || !config_data["logger"].is_object()) {
        return std::nullopt;
    }
    const auto& logger = config_data["logger"];
    if (logger.contains("min_location_length") && logger["min_location_length"].is_number_integer()) {
        return logger["min_location_length"].get<uint32_t>();
    }
    return std::nullopt;
}

auto getLoggerMinModuleLength() -> std::optional<uint32_t> {
    const auto& config_data = Config::config();
    if (!config_data.contains("logger") || !config_data["logger"].is_object()) {
        return std::nullopt;
    }
    const auto& logger = config_data["logger"];
    if (logger.contains("min_module_length") && logger["min_module_length"].is_number_integer()) {
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
    if (levels) {
        result.log_level = *levels;
    }
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

} // namespace oink_judge::config