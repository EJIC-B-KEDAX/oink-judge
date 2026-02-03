#pragma once

#include <cstdint>
#include <map>
#include <ostream>
#include <source_location>
#include <string>
#include <sys/types.h>

namespace oink_judge::logger {

class Logger {
  public:
    Logger(const Logger&) = delete;
    auto operator=(const Logger&) -> Logger& = delete;
    Logger(Logger&&) = delete;
    auto operator=(Logger&&) -> Logger& = delete;
    ~Logger();

    static auto instance() -> Logger&;

    auto setOutputStream(std::ostream& out) -> void;

    auto log(const std::string& module, int level, const std::string& message) -> void;

    auto setLogLevel(const std::string& module, int level) -> void;

    auto setColorMap(const std::map<std::string, std::string>& color_map) -> void;

    auto setMinLocationLength(uint32_t length) -> void;

    auto setMinModuleLength(uint32_t length) -> void;

    [[nodiscard]] auto getLogLevel(const std::string& module) const -> int;

    [[nodiscard]] auto getColorMap() const -> const std::map<std::string, std::string>&;

    [[nodiscard]] auto getMinLocationLength() const -> uint32_t;

    [[nodiscard]] auto getMinModuleLength() const -> uint32_t;

    static const uint32_t DEFAULT_MIN_LOCATION_LENGTH = 40;
    static const uint32_t DEFAULT_MIN_MODULE_LENGTH = 20;
    static const std::map<std::string, std::string> DEFAULT_COLOR_MAP;

  private:
    Logger();
    std::ostream* out_stream_;
    std::map<std::string, int> log_levels_;
    std::map<std::string, std::string> color_map_;
    uint32_t min_location_length_;
    uint32_t min_module_length_;
};

inline const std::map<std::string, std::string> Logger::DEFAULT_COLOR_MAP = {
    {"DEBUG", "\033[36m"},      // Cyan
    {"INFO", "\033[0m"},        // Default
    {"SUCCESS", "\033[32m"},    // Green
    {"WARNING", "\033[33m"},    // Yellow
    {"ERROR", "\033[31m"},      // Red
    {"CRITICAL", "\033[2;31m"}, // Dark Red (Dim + Red)
    {"RESET", "\033[0m"},       // Reset
    {"LOCATION", "\033[94m"},   // Bright Blue
    {"TIMESTAMP", "\033[90m"}   // Bright Black
};

enum LogType : std::uint8_t { DEBUG, INFO, SUCCESS, WARNING, ERROR, CRITICAL };

auto logMessage(const std::string& module, int level, const std::string& message, LogType type = LogType::INFO,
                uint32_t full_function_name_on_level = UINT32_MAX,
                std::source_location location = std::source_location::current()) -> void;

auto disableColors() -> void;

auto enableColors(const std::map<std::string, std::string>& color_map = Logger::DEFAULT_COLOR_MAP) -> void;

} // namespace oink_judge::logger
