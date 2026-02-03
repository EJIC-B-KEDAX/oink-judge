#include "oink_judge/logger/logger.h"

#include <iostream>
#include <map>
#include <sstream>
#include <string>

using namespace oink_judge::logger;

static auto expectTrue(bool cond, const std::string& msg, std::source_location location = std::source_location::current())
    -> bool {
    Logger::instance().setOutputStream(std::cerr);
    enableColors();
    Logger::instance().setMinLocationLength(Logger::DEFAULT_MIN_LOCATION_LENGTH);
    Logger::instance().setMinModuleLength(Logger::DEFAULT_MIN_MODULE_LENGTH);
    if (!cond) {
        logMessage("TestLogger", 1, msg, LogType::CRITICAL, 2, location);
        return false;
    }

    logMessage("TestLogger", 1, msg, LogType::SUCCESS, 2, location);
    return true;
}

static auto testLogLevelFiltering() -> bool;

static auto testColorDisableEnable() -> bool;

static auto testGettersSetters() -> bool;

int main() {
    Logger::instance().setLogLevel("TestLogger", 1);

    bool success = true;
    success &= testLogLevelFiltering();
    success &= testColorDisableEnable();
    success &= testGettersSetters();

    return success ? 0 : 1;
}

static auto testLogLevelFiltering() -> bool {
    bool success = true;

    std::ostringstream out;
    Logger::instance().setOutputStream(out);
    Logger::instance().setLogLevel("TestLogger", 1);

    logMessage("TestLogger", 1, "visible message", LogType::INFO);
    logMessage("TestLogger", 2, "hidden message", LogType::INFO);

    std::string out_str = out.str();
    success &= expectTrue(out_str.find("visible message") != std::string::npos, "level 1 message logged");
    success &= expectTrue(out_str.find("hidden message") == std::string::npos, "level 2 message not logged");
    return success;
}

static auto testColorDisableEnable() -> bool {
    bool success = true;

    std::ostringstream out;
    Logger::instance().setOutputStream(out);
    Logger::instance().setLogLevel("TestLogger", 1);

    // Ensure colors present by default (or when enabled)
    enableColors(Logger::DEFAULT_COLOR_MAP);
    out.str(std::string());
    logMessage("TestLogger", 1, "colored", LogType::INFO);
    std::string colored_out = out.str();
    success &= expectTrue(colored_out.find("\033[") != std::string::npos, "colors present after enableColors");

    // Disable colors and verify escape sequences removed
    disableColors();
    Logger::instance().setOutputStream(out);
    out.str(std::string());
    logMessage("TestLogger", 1, "nocolor", LogType::INFO);
    std::string no_color_out = out.str();
    success &= expectTrue(no_color_out.find("\033[") == std::string::npos, "no color sequences after disableColors");

    return success;
}

static auto testGettersSetters() -> bool {
    bool success = true;

    const uint32_t TEST_MIN_LOCATION_LENGTH = 30;
    const uint32_t TEST_MIN_MODULE_LENGTH = 15;

    // Test min lengths
    Logger::instance().setMinLocationLength(TEST_MIN_LOCATION_LENGTH);
    Logger::instance().setMinModuleLength(TEST_MIN_MODULE_LENGTH);
    bool get_min_location_length_correct = Logger::instance().getMinLocationLength() == TEST_MIN_LOCATION_LENGTH;
    bool get_min_module_length_correct = Logger::instance().getMinModuleLength() == TEST_MIN_MODULE_LENGTH;
    success &= expectTrue(get_min_location_length_correct, "get/set min location length");
    success &= expectTrue(get_min_module_length_correct, "get/set min module length");

    // Test color map set/get by merging custom entries into the default map
    std::map<std::string, std::string> custom_map = {{"INFO", "X"}, {"RESET", "Y"}};
    enableColors(custom_map);
    const auto& got = Logger::instance().getColorMap();
    bool maps_equal = true;
    for (const auto& [key, value] : custom_map) {
        auto iter = got.find(key);
        maps_equal &= (iter != got.end() && iter->second == value);
    }
    for (const auto& [key, value] : got) {
        auto iter = custom_map.find(key);
        maps_equal &= (iter != custom_map.end() && iter->second == value);
    }
    success &= expectTrue(maps_equal, "set/get color map");

    return success;
}
