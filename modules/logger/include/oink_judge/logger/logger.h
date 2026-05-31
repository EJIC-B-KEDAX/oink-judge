#pragma once
#include <cstdint>
#include <map>
#include <ostream>
#include <source_location>
#include <string>

namespace oink_judge::logger {

enum LogType : std::uint8_t { DEBUG, INFO, SUCCESS, WARNING, ERROR, CRITICAL };

struct LocationFormatOptions {
    uint32_t strip_params_on_level = UINT32_MAX;
    uint32_t strip_return_type_on_level = UINT32_MAX;
    uint32_t strip_namespace_on_level = UINT32_MAX;
};

class Logger {
  public:
    Logger(const Logger&) = delete;
    auto operator=(const Logger&) -> Logger& = delete;
    Logger(Logger&&) = delete;
    auto operator=(Logger&&) -> Logger& = delete;
    ~Logger();

    static auto instance() -> Logger&;

    [[nodiscard]] auto isLoggingEnabled(const std::string& module, uint32_t level) const -> bool;

    /// Checks level, formats the entry, and prints it.
    auto log(const std::string& module, const std::string& message, LogType type, uint32_t level, std::source_location location)
        -> void;

    /// Writes a pre-formatted string directly to the output stream (no level check).
    auto print(const std::string& message) -> void;

    auto setOutputStream(std::ostream& out) -> void;

    auto setLogLevel(const std::string& module, uint32_t level) -> void;

    auto setColorMap(const std::map<std::string, std::string>& color_map) -> void;

    auto setMinLocationLength(uint32_t length) -> void;

    auto setMinModuleLength(uint32_t length) -> void;

    auto setTimestampFormat(const std::string& format) -> void;

    auto setLocationFormatOptions(const std::string& module, const LocationFormatOptions& options) -> void;

    [[nodiscard]] auto getLogLevel(const std::string& module) const -> uint32_t;

    [[nodiscard]] auto getColorMap() const -> const std::map<std::string, std::string>&;

    [[nodiscard]] auto getMinLocationLength() const -> uint32_t;

    [[nodiscard]] auto getMinModuleLength() const -> uint32_t;

    [[nodiscard]] auto getTimestampFormat() const -> const std::string&;

    [[nodiscard]] auto getLocationFormatOptions(const std::string& module) const -> LocationFormatOptions;

    static constexpr uint32_t DEFAULT_MIN_LOCATION_LENGTH = 40;
    static constexpr uint32_t DEFAULT_MIN_MODULE_LENGTH = 20;
    static constexpr auto DEFAULT_TIMESTAMP_FORMAT = "%Y-%m-%d %H:%M:%S";
    static const std::map<std::string, std::string> DEFAULT_COLOR_MAP;

  private:
    Logger();

    [[nodiscard]] auto safeGetColor(const std::string& key) const -> std::string;
    [[nodiscard]] auto formatTimestamp() const -> std::string;
    [[nodiscard]] auto formatLogType(LogType type) const -> std::string;
    [[nodiscard]] auto formatLocation(const std::string& module, std::source_location location, uint32_t level) const
        -> std::string;
    [[nodiscard]] auto formatModule(const std::string& module) const -> std::string;
    [[nodiscard]] auto formatEntry(const std::string& module, const std::string& message, LogType type, uint32_t level,
                                   std::source_location location) const -> std::string;

    std::ostream* out_stream_;
    std::map<std::string, uint32_t> log_levels_;
    std::map<std::string, std::string> color_map_;
    uint32_t min_location_length_;
    uint32_t min_module_length_;
    std::string timestamp_format_;
    std::map<std::string, LocationFormatOptions> location_format_options_;
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

auto disableColors() -> void;

auto enableColors(const std::map<std::string, std::string>& color_map = Logger::DEFAULT_COLOR_MAP) -> void;

auto logMessage(const std::string& module, const std::string& message, LogType type = LogType::INFO, uint32_t level = 1,
                std::source_location location = std::source_location::current()) -> void;

auto logDebug(const std::string& module, const std::string& message, uint32_t level = 1,
              std::source_location location = std::source_location::current()) -> void;

auto logInfo(const std::string& module, const std::string& message, uint32_t level = 1,
             std::source_location location = std::source_location::current()) -> void;

auto logSuccess(const std::string& module, const std::string& message, uint32_t level = 1,
                std::source_location location = std::source_location::current()) -> void;

auto logWarning(const std::string& module, const std::string& message, uint32_t level = 1,
                std::source_location location = std::source_location::current()) -> void;

auto logError(const std::string& module, const std::string& message, uint32_t level = 1,
              std::source_location location = std::source_location::current()) -> void;

auto logCritical(const std::string& module, const std::string& message, uint32_t level = 1,
                 std::source_location location = std::source_location::current()) -> void;

} // namespace oink_judge::logger
