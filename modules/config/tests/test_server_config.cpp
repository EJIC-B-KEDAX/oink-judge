#include "oink_judge/config/config.h"
#include "oink_judge/config/server_config_utils.h"

#include <filesystem>
#include <oink_judge/logger/logger.h>
#include <source_location>
#include <string>

using namespace oink_judge::config;
using namespace oink_judge::logger;
namespace fs = std::filesystem;

static auto expectTrue(bool cond, const std::string& msg, std::source_location location = std::source_location::current())
    -> bool {
    if (!cond) {
        logMessage("TestConfig", 1, msg, LogType::CRITICAL, 2, location);
        return false;
    }

    logMessage("TestConfig", 1, msg, LogType::SUCCESS, 2, location);
    return true;
}

const fs::path RESOURCES = fs::path("resources") / "test_server_config";

static auto setConfig(const fs::path& config_path, const fs::path& credentials_path) -> void {
    Config::setConfigFilePath(config_path);
    Config::setCredentialsFilePath(credentials_path);
    Config::reloadData();
}

// Forward declarations
static auto testMyPort() -> bool;
static auto testConnectionHandlerType() -> bool;

int main() {
    namespace fs = std::filesystem;
    fs::path test_dir = fs::path(__FILE__).parent_path();
    fs::current_path(test_dir);

    Logger::instance().setLogLevel("TestConfig", 1);

    // Run grouped tests implemented as helper functions
    bool success = true;
    success &= testMyPort();
    success &= testConnectionHandlerType();
    return success ? 0 : 1;
}

static auto testMyPort() -> bool {
    bool success = true;

    setConfig(RESOURCES / "good_config.json", RESOURCES / "good_credentials.json");

    const int EXPECTED_MY_PORT = 9000;
    auto myp = getMyPort();
    success &= expectTrue(myp.has_value() && *myp == EXPECTED_MY_PORT, "getMyPort returns port");

    setConfig(RESOURCES / "bad_missing_my_port.json", RESOURCES / "good_credentials.json");

    myp = getMyPort();
    success &= expectTrue(!myp.has_value(), "getMyPort returns nullopt for missing my_port");

    return success;
}

static auto testConnectionHandlerType() -> bool {
    bool success = true;

    setConfig(RESOURCES / "good_config.json", RESOURCES / "good_credentials.json");

    const std::string EXPECTED_CONNECTION_HANDLER = "epoll";
    auto cht = getConnectionHandlerType();
    success &= expectTrue(cht.has_value() && *cht == EXPECTED_CONNECTION_HANDLER,
                          "getConnectionHandlerType returns connection_handler_type");

    setConfig(RESOURCES / "bad_missing_connection_handler.json", RESOURCES / "good_credentials.json");

    cht = getConnectionHandlerType();
    success &= expectTrue(!cht.has_value(), "getConnectionHandlerType returns nullopt for missing connection_handler_type");

    return success;
}
