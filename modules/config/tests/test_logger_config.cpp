#include "oink_judge/config/config.h"
#include "oink_judge/config/logger_config_utils.h"

#include <filesystem>
#include <oink_judge/logger/logger.h>
#include <source_location>
#include <string>
#include <sys/types.h>

using namespace oink_judge::config;
using namespace oink_judge::logger;
namespace fs = std::filesystem;

static auto expectTrue(bool cond, const std::string& msg, std::source_location location = std::source_location::current())
    -> bool {
    if (!cond) {
        logMessage("TestLoggerConfig", 1, msg, LogType::CRITICAL, 2, location);
        return false;
    }

    logMessage("TestLoggerConfig", 1, msg, LogType::SUCCESS, 2, location);
    return true;
}

const fs::path RESOURCES = fs::path("resources") / "test_logger_config";

static auto setConfig(const fs::path& config_path, const fs::path& credentials_path) -> void {
    Config::setConfigFilePath(config_path);
    Config::setCredentialsFilePath(credentials_path);
    Config::reloadData();
}

static auto testOutputStream() -> bool;
static auto testLogLevelsAndDefault() -> bool;
static auto testColorMap() -> bool;
static auto testMinLengths() -> bool;
static auto testMissingLogger() -> bool;

int main() {
    fs::path test_dir = fs::path(__FILE__).parent_path();
    fs::current_path(test_dir);

    Logger::instance().setLogLevel("TestLoggerConfig", 1);

    bool success = true;
    success &= testOutputStream();
    success &= testLogLevelsAndDefault();
    success &= testColorMap();
    success &= testMinLengths();
    success &= testMissingLogger();

    return success ? 0 : 1;
}

static auto testOutputStream() -> bool {
    bool success = true;

    setConfig(RESOURCES / "good_config.json", RESOURCES / "good_credentials.json");

    auto out = getLoggerOutputStream();
    success &= expectTrue(out.has_value() && *out == "stdout", "getLoggerOutputStream returns stdout");

    return success;
}

static auto testLogLevelsAndDefault() -> bool {
    bool success = true;

    setConfig(RESOURCES / "good_config.json", RESOURCES / "good_credentials.json");

    auto all = getAllLoggerLogLevels();
    if (!expectTrue(all.has_value() && all->size() == 3, "getAllLoggerLogLevels size")) {
        return false;
    }

    const uint32_t EXPECTED_DEFAULT_LEVEL = 3;
    const uint32_t EXPECTED_MODULE_A_LEVEL = 2;
    const uint32_t EXPECTED_MODULE_B_LEVEL = 5;

    success &= expectTrue(all->at("moduleA") == EXPECTED_MODULE_A_LEVEL, "moduleA level");
    success &= expectTrue(all->at("moduleB") == EXPECTED_MODULE_B_LEVEL, "moduleB level");
    success &= expectTrue(all->at("default") == EXPECTED_DEFAULT_LEVEL, "default level");

    auto lvl = getLoggerLogLevel("moduleA");
    success &= expectTrue(lvl.has_value() && *lvl == EXPECTED_MODULE_A_LEVEL, "getLoggerLogLevel for moduleA");

    lvl = getLoggerLogLevel("nonexistent");
    success &= expectTrue(!lvl.has_value(), "getLoggerLogLevel falls back to default");

    return success;
}

static auto testColorMap() -> bool {
    bool success = true;

    setConfig(RESOURCES / "good_config.json", RESOURCES / "good_credentials.json");

    auto cmap = getLoggerColorMap();
    if (!expectTrue(cmap.has_value() && cmap->size() == 2, "getLoggerColorMap size")) {
        return false;
    }

    success &= expectTrue(cmap->at("INFO") == "green", "color for INFO");
    success &= expectTrue(cmap->at("ERROR") == "red", "color for ERROR");

    return success;
}

static auto testMinLengths() -> bool {
    bool success = true;

    setConfig(RESOURCES / "good_config.json", RESOURCES / "good_credentials.json");

    const uint32_t EXPECTED_MIN_LOCATION_LENGTH = 10;
    const uint32_t EXPECTED_MIN_MODULE_LENGTH = 8;

    auto min_loc = getLoggerMinLocationLength();
    success &= expectTrue(min_loc.has_value() && *min_loc == EXPECTED_MIN_LOCATION_LENGTH, "min_location_length");

    auto min_mod = getLoggerMinModuleLength();
    success &= expectTrue(min_mod.has_value() && *min_mod == EXPECTED_MIN_MODULE_LENGTH, "min_module_length");

    return success;
}

static auto testMissingLogger() -> bool {
    bool success = true;

    setConfig(RESOURCES / "no_logger.json", RESOURCES / "good_credentials.json");

    auto out = getLoggerOutputStream();
    success &= expectTrue(!out.has_value(), "getLoggerOutputStream returns nullopt when logger missing");

    auto all = getAllLoggerLogLevels();
    success &= expectTrue(!all.has_value(), "getAllLoggerLogLevels returns nullopt when logger missing");

    auto cfg = getLoggerConfig();
    success &= expectTrue(!cfg.has_value(), "getLoggerConfig returns nullopt when logger missing");

    return success;
}
