#include "oink_judge/logger/logger.h"

#include "oink_judge/logger/python_logger.h"

#include <Python.h>

#include <chrono>
#include <cstddef>
#include <cstdint>
#include <ctime>
#include <iomanip>
#include <iostream>
#include <sstream>

namespace oink_judge::logger {

Logger::~Logger() = default;

auto Logger::instance() -> Logger& {
    static Logger instance;
    return instance;
}

auto Logger::isLoggingEnabled(const std::string& module, uint32_t level) const -> bool {
    return (level <= getLogLevel("default") && !log_levels_.contains(module)) || level <= getLogLevel(module);
}

auto Logger::log(const std::string& module, const std::string& message, LogType type, uint32_t level,
                 std::source_location location) -> void {
    if (!isLoggingEnabled(module, level)) {
        return;
    }
    print(formatEntry(module, message, type, level, location));
}

auto Logger::print(const std::string& message) -> void { (*out_stream_) << message; }

auto Logger::setOutputStream(std::ostream& out) -> void { out_stream_ = &out; }

auto Logger::setLogLevel(const std::string& module, uint32_t level) -> void { log_levels_[module] = level; }

auto Logger::setColorMap(const std::map<std::string, std::string>& color_map) -> void { color_map_ = color_map; }

auto Logger::setMinLocationLength(uint32_t length) -> void { min_location_length_ = length; }

auto Logger::setMinModuleLength(uint32_t length) -> void { min_module_length_ = length; }

auto Logger::setTimestampFormat(const std::string& format) -> void { timestamp_format_ = format; }

auto Logger::setLocationFormatOptions(const std::string& module, const LocationFormatOptions& options) -> void {
    location_format_options_[module] = options;
}

auto Logger::getLogLevel(const std::string& module) const -> uint32_t {
    auto it = log_levels_.find(module);
    if (it != log_levels_.end()) {
        return it->second;
    }
    auto default_it = log_levels_.find("default");
    if (default_it != log_levels_.end()) {
        return default_it->second;
    }
    return 0;
}

auto Logger::getColorMap() const -> const std::map<std::string, std::string>& { return color_map_; }

auto Logger::getMinLocationLength() const -> uint32_t { return min_location_length_; }

auto Logger::getMinModuleLength() const -> uint32_t { return min_module_length_; }

auto Logger::getTimestampFormat() const -> const std::string& { return timestamp_format_; }

auto Logger::getLocationFormatOptions(const std::string& module) const -> LocationFormatOptions {
    auto it = location_format_options_.find(module);
    if (it != location_format_options_.end()) {
        return it->second;
    }
    auto default_it = location_format_options_.find("default");
    if (default_it != location_format_options_.end()) {
        return default_it->second;
    }
    return {};
}

Logger::Logger()
    : out_stream_(&std::cerr), min_location_length_(DEFAULT_MIN_LOCATION_LENGTH), min_module_length_(DEFAULT_MIN_MODULE_LENGTH),
      color_map_(DEFAULT_COLOR_MAP), timestamp_format_(DEFAULT_TIMESTAMP_FORMAT) {}

// --- private formatting methods ---

auto Logger::safeGetColor(const std::string& key) const -> std::string {
    auto iter = color_map_.find(key);
    if (iter != color_map_.end()) {
        return iter->second;
    }
    return "";
}

auto Logger::formatTimestamp() const -> std::string {
    auto now = std::chrono::system_clock::now();
    auto time_t_now = std::chrono::system_clock::to_time_t(now);
    auto ms = std::chrono::duration_cast<std::chrono::milliseconds>(now.time_since_epoch()) % 1000; // NOLINT

    std::tm tm_now{};
    localtime_r(&time_t_now, &tm_now);

    std::ostringstream timestamp;
    timestamp << std::put_time(&tm_now, timestamp_format_.c_str()) << '.' << std::setfill('0') << std::setw(3) << ms.count();
    return timestamp.str();
}

auto Logger::formatLogType(LogType type) const -> std::string {
    switch (type) {
    case LogType::DEBUG:
        return safeGetColor("DEBUG") + "DEBUG" + safeGetColor("RESET");
    case LogType::INFO:
        return safeGetColor("INFO") + "INFO " + safeGetColor("RESET");
    case LogType::SUCCESS:
        return safeGetColor("SUCCESS") + "SUCC " + safeGetColor("RESET");
    case LogType::WARNING:
        return safeGetColor("WARNING") + "WARN " + safeGetColor("RESET");
    case LogType::ERROR:
        return safeGetColor("ERROR") + "ERROR" + safeGetColor("RESET");
    case LogType::CRITICAL:
        return safeGetColor("CRITICAL") + "CRIT " + safeGetColor("RESET");
    }
    return "";
}

auto Logger::formatLocation(const std::string& module, std::source_location location, uint32_t level) const -> std::string {
    LocationFormatOptions options = getLocationFormatOptions(module);

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
    if (level < options.strip_params_on_level) {
        size_t paren_pos = function_name.find_first_of('(');
        if (paren_pos != std::string::npos) {
            function_name = function_name.substr(0, paren_pos);
        }
    }
    if (level < options.strip_return_type_on_level) {
        size_t space_pos = function_name.find_last_of(' ');
        if (space_pos != std::string::npos) {
            function_name = function_name.substr(space_pos + 1);
        }
    }
    if (level < options.strip_namespace_on_level) {
        size_t colons_pos = function_name.rfind("::");
        if (colons_pos != std::string::npos) {
            function_name = function_name.substr(colons_pos + 2);
        }
    }

    std::string location_str =
        file_name + "." + function_name + ":" + std::to_string(location.line()); // TODO: make configurable format
    while (static_cast<uint32_t>(location_str.length()) < min_location_length_) {
        location_str += " ";
    }

    return safeGetColor("LOCATION") + location_str + safeGetColor("RESET");
}

auto Logger::formatModule(const std::string& module) const -> std::string {
    std::string mod = module;
    while (static_cast<uint32_t>(mod.length()) < min_module_length_) {
        mod += " ";
    }
    return mod;
}

auto Logger::formatEntry(const std::string& module, const std::string& message, LogType type, uint32_t level,
                         std::source_location location) const -> std::string {
    std::string entry = safeGetColor("TIMESTAMP") + "[" + formatTimestamp() + "]" + safeGetColor("RESET") + " | ";
    entry += formatLogType(type) + " | ";
    entry += formatLocation(module, location, level) + " | ";
    entry += formatModule(module) + " | ";
    entry += type == LogType::CRITICAL ? safeGetColor("CRITICAL") : "";
    entry += message + (type == LogType::CRITICAL ? safeGetColor("RESET") : "") + "\n";
    return entry;
}

// --- free functions ---

auto disableColors() -> void {
    auto& logger = Logger::instance();
    std::map<std::string, std::string> no_color_map;
    for (const auto& [key, value] : logger.getColorMap()) {
        no_color_map[key] = "";
    }
    logger.setColorMap(no_color_map);
}

auto enableColors(const std::map<std::string, std::string>& color_map) -> void { Logger::instance().setColorMap(color_map); }

auto logMessage(const std::string& module, const std::string& message, LogType type, uint32_t level,
                std::source_location location) -> void {
    if (Py_IsInitialized() != 0) {
        PythonLogger::log(module, message, type, level, location);
    } else {
        Logger::instance().log(module, message, type, level, location);
    }
}

auto logDebug(const std::string& module, const std::string& message, uint32_t level, std::source_location location) -> void {
    logMessage(module, message, LogType::DEBUG, level, location);
}

auto logInfo(const std::string& module, const std::string& message, uint32_t level, std::source_location location) -> void {
    logMessage(module, message, LogType::INFO, level, location);
}

auto logSuccess(const std::string& module, const std::string& message, uint32_t level, std::source_location location) -> void {
    logMessage(module, message, LogType::SUCCESS, level, location);
}

auto logWarning(const std::string& module, const std::string& message, uint32_t level, std::source_location location) -> void {
    logMessage(module, message, LogType::WARNING, level, location);
}

auto logError(const std::string& module, const std::string& message, uint32_t level, std::source_location location) -> void {
    logMessage(module, message, LogType::ERROR, level, location);
}

auto logCritical(const std::string& module, const std::string& message, uint32_t level, std::source_location location) -> void {
    logMessage(module, message, LogType::CRITICAL, level, location);
}

} // namespace oink_judge::logger
