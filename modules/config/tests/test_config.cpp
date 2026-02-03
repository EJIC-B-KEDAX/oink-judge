#include "oink_judge/config/config.h"

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

const fs::path RESOURCES = fs::path("resources") / "test_config";

static auto setConfig(const fs::path& config_path, const fs::path& credentials_path) -> void {
    Config::setConfigFilePath(config_path);
    Config::setCredentialsFilePath(credentials_path);
    Config::reloadData();
}

// Forward declarations for helper tests
static auto testDirectoryConfig() -> bool;
static auto testDatabaseConfig() -> bool;
static auto testServerPortAndHost() -> bool;
static auto testSessionType() -> bool;
static auto testStartMessage() -> bool;
static auto testTiming() -> bool;
static auto testMalformedConfigJSON() -> bool;
static auto testMissingConfigFile() -> bool;

int main() {
    fs::path test_dir = fs::path(__FILE__).parent_path();
    fs::current_path(test_dir);

    Logger::instance().setLogLevel("TestConfig", 1);

    // Run grouped tests implemented as helper functions
    bool success = true;
    success &= testDirectoryConfig();
    success &= testDatabaseConfig();
    success &= testServerPortAndHost();
    success &= testSessionType();
    success &= testStartMessage();
    success &= testTiming();
    success &= testMalformedConfigJSON();
    success &= testMissingConfigFile();

    return success ? 0 : 1;
}

static auto testDirectoryConfig() -> bool {
    bool success = true;

    setConfig(RESOURCES / "good_config.json", RESOURCES / "good_credentials.json");

    const fs::path EXPECTED_PROBLEMS_DIR = RESOURCES / "problems";
    auto dir = getDirectoryPath("problems");
    success &=
        expectTrue(dir.has_value() && std::filesystem::absolute(dir.value()) == std::filesystem::absolute(EXPECTED_PROBLEMS_DIR),
                   "getDirectoryPath returns problems dir");

    auto missing_dir = getDirectoryPath("non_existent_key");
    success &= expectTrue(!missing_dir.has_value(), "getDirectoryPath returns nullopt for missing key");
    return success;
}

static auto testDatabaseConfig() -> bool {
    bool success = true;

    setConfig(RESOURCES / "good_config.json", RESOURCES / "good_credentials.json");

    const std::string EXPECTED_DB_HOST = "localhost";
    const int EXPECTED_DB_PORT = 5432;
    const std::string EXPECTED_DB_USER = "user";
    const std::string EXPECTED_DB_PASSWORD = "secret";
    const std::string EXPECTED_DB_NAME = "db";

    auto db_config = getDatabaseConfig();
    if (!expectTrue(db_config.has_value(), "getDatabaseConfig returns value")) {
        return false;
    }

    success &= expectTrue(db_config->host == EXPECTED_DB_HOST, "db.host");
    success &= expectTrue(db_config->port == EXPECTED_DB_PORT, "db.port");
    success &= expectTrue(db_config->username == EXPECTED_DB_USER, "db.username");
    success &= expectTrue(db_config->password == EXPECTED_DB_PASSWORD, "db.password");
    success &= expectTrue(db_config->database_name == EXPECTED_DB_NAME, "db.name");

    setConfig(RESOURCES / "bad_missing_db.json", RESOURCES / "good_credentials.json");

    // Missing database key in config -> getDatabaseConfig should return nullopt
    db_config = getDatabaseConfig();
    success &= expectTrue(!db_config.has_value(), "getDatabaseConfig returns nullopt when database key missing");

    setConfig(RESOURCES / "good_config.json", RESOURCES / "bad_missing_credentials.json");

    // Missing password in credentials -> getDatabaseConfig should return nullopt
    db_config = getDatabaseConfig();
    success &= expectTrue(!db_config.has_value(), "getDatabaseConfig returns nullopt when credentials password missing");

    return success;
}

static auto testServerPortAndHost() -> bool {
    bool success = true;

    setConfig(RESOURCES / "good_config.json", RESOURCES / "good_credentials.json");

    const int EXPECTED_SERVER_PORT = 8080;
    const std::string EXPECTED_SERVER_HOSTNAME = "localhost";

    auto port = getServerPort("serverA");
    success &= expectTrue(port.has_value() && *port == EXPECTED_SERVER_PORT, "getServerPort returns port");

    port = getServerPort("non_existent_server");
    success &= expectTrue(!port.has_value(), "getServerPort returns nullopt for missing server");

    auto host = getServerHostname("serverA");
    success &= expectTrue(host.has_value() && *host == EXPECTED_SERVER_HOSTNAME, "getServerHostname returns hostname");

    host = getServerHostname("non_existent_server");
    success &= expectTrue(!host.has_value(), "getServerHostname returns nullopt for missing server");

    return success;
}

static auto testSessionType() -> bool {
    bool success = true;

    setConfig(RESOURCES / "good_config.json", RESOURCES / "good_credentials.json");

    const std::string EXPECTED_SESSION_TYPE = "interactive";

    auto session_type = getSessionType("testSession");
    success &= expectTrue(session_type.has_value() && *session_type == EXPECTED_SESSION_TYPE, "getSessionType returns type");

    session_type = getSessionType("non_existent_session");
    success &= expectTrue(!session_type.has_value(), "getSessionType returns nullopt for missing session");

    return success;
}

static auto testStartMessage() -> bool {
    bool success = true;

    setConfig(RESOURCES / "good_config.json", RESOURCES / "good_credentials.json");

    const std::string EXPECTED_START_MESSAGE = "Welcome";

    auto start_message = getStartMessage("testSession");
    success &=
        expectTrue(start_message.has_value() && *start_message == EXPECTED_START_MESSAGE, "getStartMessage returns message");

    start_message = getStartMessage("non_existent_session");
    success &= expectTrue(!start_message.has_value(), "getStartMessage returns nullopt for missing session");

    return success;
}

static auto testTiming() -> bool {
    bool success = true;

    setConfig(RESOURCES / "good_config.json", RESOURCES / "good_credentials.json");

    const double EXPECTED_TIMING = 1.5;
    const double TOLERANCE = 1e-9;

    auto timing = getTiming("short");
    success &=
        expectTrue(timing.has_value() && std::abs(timing->count() - EXPECTED_TIMING) < TOLERANCE, "getTiming returns timing");

    timing = getTiming("non_existent_timing");
    success &= expectTrue(!timing.has_value(), "getTiming returns nullopt for missing timing");

    return success;
}

static auto testMalformedConfigJSON() -> bool {
    Config::setConfigFilePath(RESOURCES / "bad_malformed.json");
    Config::setCredentialsFilePath(RESOURCES / "good_credentials.json");

    // Malformed config JSON -> reloadData should throw (parse error)
    bool threw = false;
    try {
        Config::reloadData();
    } catch (...) {
        threw = true;
    }
    return expectTrue(threw, "reloadData throws on malformed JSON");
}

static auto testMissingConfigFile() -> bool {
    Config::setConfigFilePath(RESOURCES / "non_existent.json");
    Config::setCredentialsFilePath(RESOURCES / "good_credentials.json");

    // Missing config file -> reloadData should throw when file not found
    bool threw = false;
    try {
        Config::reloadData();
    } catch (...) {
        threw = true;
    }
    return expectTrue(threw, "reloadData throws when config file missing");
}
