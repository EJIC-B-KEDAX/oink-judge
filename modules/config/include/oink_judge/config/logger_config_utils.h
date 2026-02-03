#pragma once
#include <cstdint>
#include <map>
#include <optional>
#include <string>

namespace oink_judge::config {

struct LoggerConfig {
    std::string output_stream; // stderr by default
    std::map<std::string, uint32_t> log_level;
    std::optional<std::map<std::string, std::string>> color_map;
    std::optional<uint32_t> min_location_length;
    std::optional<uint32_t> min_module_length;
};

auto getLoggerConfig() -> std::optional<LoggerConfig>;

auto getLoggerOutputStream() -> std::optional<std::string>;

auto getLoggerLogLevel(const std::string& module) -> std::optional<uint32_t>;

auto getAllLoggerLogLevels() -> std::optional<std::map<std::string, uint32_t>>;

auto getLoggerColorMap() -> std::optional<std::map<std::string, std::string>>;

auto getLoggerMinLocationLength() -> std::optional<uint32_t>;

auto getLoggerMinModuleLength() -> std::optional<uint32_t>;

} // namespace oink_judge::config
