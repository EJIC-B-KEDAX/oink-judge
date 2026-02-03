#include "oink_judge/logger/logger.h"

#include <chrono>
#include <cstddef>
#include <iostream>

namespace oink_judge::logger {

namespace {

auto safeGetColor(const std::map<std::string, std::string>& color_map, const std::string& key) -> std::string {
    auto iter = color_map.find(key);
    if (iter != color_map.end()) {
        return iter->second;
    }
    return "";
}

} // namespace

Logger::~Logger() = default;

auto Logger::instance() -> Logger& {
    static Logger instance;
    return instance;
}

auto Logger::setOutputStream(std::ostream& out) -> void { out_stream_ = &out; }

auto Logger::log(const std::string& module, int level, const std::string& message) -> void {
    auto it = log_levels_.find(module);
    if (it == log_levels_.end() || level > it->second) {
        return; // Log level too big, ignore message
    }

    (*out_stream_) << message;
}

auto Logger::setLogLevel(const std::string& module, int level) -> void { log_levels_[module] = level; }

auto Logger::setColorMap(const std::map<std::string, std::string>& color_map) -> void { color_map_ = color_map; }

auto Logger::setMinLocationLength(uint32_t length) -> void { min_location_length_ = length; }

auto Logger::setMinModuleLength(uint32_t length) -> void { min_module_length_ = length; }

auto Logger::getLogLevel(const std::string& module) const -> int {
    auto it = log_levels_.find(module);
    if (it != log_levels_.end()) {
        return it->second;
    }
    return 0; // Default log level if not set
}

auto Logger::getColorMap() const -> const std::map<std::string, std::string>& { return color_map_; }

auto Logger::getMinLocationLength() const -> uint32_t { return min_location_length_; }

auto Logger::getMinModuleLength() const -> uint32_t { return min_module_length_; }

Logger::Logger()
    : out_stream_(&std::cerr), min_location_length_(DEFAULT_MIN_LOCATION_LENGTH), min_module_length_(DEFAULT_MIN_MODULE_LENGTH),
      color_map_(DEFAULT_COLOR_MAP) {}

auto logMessage(const std::string& module, int level, const std::string& message, LogType type,
                uint32_t full_function_name_on_level, std::source_location location) -> void {

    const std::map<std::string, std::string>& color_map = Logger::instance().getColorMap();

    auto now = std::chrono::system_clock::now();
    auto time_t_now = std::chrono::system_clock::to_time_t(now);
    auto ms = std::chrono::duration_cast<std::chrono::milliseconds>(now.time_since_epoch()) % 1000; // NOLINT

    std::tm tm_now{};
    localtime_r(&time_t_now, &tm_now);

    std::ostringstream timestamp;
    timestamp << std::put_time(&tm_now, "%Y-%m-%d %H:%M:%S") << '.' << std::setfill('0') << std::setw(3) << ms.count();

    std::string log_entry =
        safeGetColor(color_map, "TIMESTAMP") + "[" + timestamp.str() + "]" + safeGetColor(color_map, "RESET") + " | ";

    switch (type) {
    case LogType::DEBUG:
        log_entry += safeGetColor(color_map, "DEBUG") + "DEBUG" + safeGetColor(color_map, "RESET") + " | ";
        break;
    case LogType::INFO:
        log_entry += safeGetColor(color_map, "INFO") + "INFO " + safeGetColor(color_map, "RESET") + " | ";
        break;
    case LogType::SUCCESS:
        log_entry += safeGetColor(color_map, "SUCCESS") + "SUCC " + safeGetColor(color_map, "RESET") + " | ";
        break;
    case LogType::WARNING:
        log_entry += safeGetColor(color_map, "WARNING") + "WARN " + safeGetColor(color_map, "RESET") + " | ";
        break;
    case LogType::ERROR:
        log_entry += safeGetColor(color_map, "ERROR") + "ERROR" + safeGetColor(color_map, "RESET") + " | ";
        break;
    case LogType::CRITICAL:
        log_entry += safeGetColor(color_map, "CRITICAL") + "CRIT " + safeGetColor(color_map, "RESET") + " | ";
        break;
    }

    std::string file_name = location.file_name();
    size_t last_slash_pos = file_name.find_last_of("/\\");
    if (last_slash_pos != std::string::npos) {
        file_name = file_name.substr(last_slash_pos + 1);
    }
    size_t last_dot_pos = file_name.find_last_of('.');
    if (last_dot_pos != std::string::npos) {
        file_name = file_name.substr(0, last_dot_pos);
    }

    std::string function_name = location.function_name();
    if (level < full_function_name_on_level) {
        size_t paren_pos = function_name.find_first_of('(');
        if (paren_pos != std::string::npos) {
            function_name = function_name.substr(0, paren_pos);
        }
        size_t space_pos = function_name.find_last_of(' ');
        if (space_pos != std::string::npos) {
            function_name = function_name.substr(space_pos + 1);
        }
        size_t colons_pos = function_name.find_last_of("::");
        if (colons_pos != std::string::npos) {
            function_name = function_name.substr(colons_pos + 1);
        }
    }

    std::string location_str = file_name + "." + function_name + ":" + std::to_string(location.line());
    while (static_cast<int>(location_str.length()) < Logger::instance().getMinLocationLength()) {
        location_str += " ";
    }

    log_entry += safeGetColor(color_map, "LOCATION") + location_str + safeGetColor(color_map, "RESET") + " | ";

    std::string mod = module;
    while (static_cast<int>(mod.length()) < Logger::instance().getMinModuleLength()) {
        mod += " ";
    }

    log_entry += mod + " | ";

    log_entry += type == LogType::CRITICAL ? safeGetColor(color_map, "CRITICAL") : "";

    log_entry += message + "\n" + (type == LogType::CRITICAL ? safeGetColor(color_map, "RESET") : "");
    Logger::instance().log(module, level, log_entry);
}

auto disableColors() -> void {
    auto& logger = Logger::instance();
    std::map<std::string, std::string> no_color_map;
    for (const auto& [key, value] : logger.getColorMap()) {
        no_color_map[key] = "";
    }
    logger.setColorMap(no_color_map);
}

auto enableColors(const std::map<std::string, std::string>& color_map) -> void { Logger::instance().setColorMap(color_map); }

} // namespace oink_judge::logger
