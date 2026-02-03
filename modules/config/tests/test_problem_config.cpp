#include "oink_judge/config/config.h"
#include "oink_judge/config/problem_config_utils.h"

#include <filesystem>
#include <oink_judge/logger/logger.h>
#include <source_location>
#include <string>

using namespace oink_judge::config;
using namespace oink_judge::problem_config;
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

const fs::path RESOURCES = fs::path("resources") / "test_problem_config";

static auto setConfig(const fs::path& config_path, const fs::path& credentials_path) -> void {
    Config::setConfigFilePath(config_path);
    Config::setCredentialsFilePath(credentials_path);
    Config::reloadData();
}

// Forward declarations
static auto testAllTestNames() -> bool;
static auto testProblemBuilderName() -> bool;
static auto testGetTestConfig() -> bool;
static auto testNoProblemsDir() -> bool;
static auto testMalformedProblemXML() -> bool;
static auto testNoProblemConfig() -> bool;

int main() {
    fs::path test_dir = fs::path(__FILE__).parent_path();
    fs::current_path(test_dir);

    Logger::instance().setLogLevel("TestConfig", 1);

    // Run grouped tests implemented as helper functions
    bool success = true;
    success &= testAllTestNames();
    success &= testProblemBuilderName();
    success &= testGetTestConfig();
    success &= testNoProblemsDir();
    success &= testMalformedProblemXML();
    success &= testNoProblemConfig();
    return success ? 0 : 1;
}

static auto testAllTestNames() -> bool {
    bool success = true;

    setConfig(RESOURCES / "good_config.json", RESOURCES / "good_credentials.json");

    const std::string EXPECTED_T1 = "t1";
    const std::string EXPECTED_T2 = "t2";

    auto all_tests = getAllTestNames("problem_two_tests");
    if (!expectTrue(all_tests.has_value() && all_tests->size() == 2, "getAllTestNames size")) {
        success = false;
    } else {
        success &= expectTrue((*all_tests)[0] == EXPECTED_T1 && (*all_tests)[1] == EXPECTED_T2, "getAllTestNames content");
    }

    auto empty_tests = getAllTestNames("problem_no_tests");
    success &= expectTrue(empty_tests.has_value() && empty_tests->empty(), "getAllTestNames empty problem");

    auto no_node_tests = getAllTestNames("problem_no_node_tests");
    success &= expectTrue(!no_node_tests.has_value(), "getAllTestNames returns nullopt for problem with no tests node");

    auto missing_tests = getAllTestNames("non_existent_problem");
    success &= expectTrue(!missing_tests.has_value(), "getAllTestNames returns nullopt for missing problem");

    return success;
}

static auto testProblemBuilderName() -> bool {
    bool success = true;

    setConfig(RESOURCES / "good_config.json", RESOURCES / "good_credentials.json");

    const std::string EXPECTED_BUILDER = "default";

    auto builder = getProblemBuilderName("problem_two_tests");
    success &= expectTrue(builder.has_value() && *builder == EXPECTED_BUILDER, "getProblemBuilderName");

    builder = getProblemBuilderName("problem_no_builder");
    success &= expectTrue(!builder.has_value(), "getProblemBuilderName returns nullopt for missing builder");

    return success;
}

static auto testGetTestConfig() -> bool {
    bool success = true;

    setConfig(RESOURCES / "good_config.json", RESOURCES / "good_credentials.json");

    const std::string EXPECTED_TEST_NAME = "t2";

    auto tnode = getTestConfig("problem_two_tests", "t2");
    if (!expectTrue(tnode.has_value(), "getTestConfig returns node")) {
        success = false;
    } else {
        success &=
            expectTrue(std::string(tnode->attribute("name").as_string()) == EXPECTED_TEST_NAME, "test node attribute name");
    }

    tnode = getTestConfig("problem_two_tests", "non_existent_test");
    success &= expectTrue(!tnode.has_value(), "getTestConfig returns nullopt for missing test");

    return success;
}

static auto testNoProblemsDir() -> bool {
    bool success = true;

    setConfig(RESOURCES / "bad_no_problems.json", RESOURCES / "good_credentials.json");

    auto all = getAllTestNames("problem_two_tests");
    success &= expectTrue(!all.has_value(), "getAllTestNames returns nullopt when problems dir missing");
    return success;
}

static auto testMalformedProblemXML() -> bool {
    bool success = true;

    setConfig(RESOURCES / "good_config.json", RESOURCES / "good_credentials.json");

    auto all = getAllTestNames("problem_bad_xml");
    success &= expectTrue(!all.has_value(), "getAllTestNames returns nullopt when problem XML malformed");
    return success;
}

static auto testNoProblemConfig() -> bool {
    bool success = true;

    setConfig(RESOURCES / "good_config.json", RESOURCES / "good_credentials.json");

    auto all = getAllTestNames("problem_no_config");
    success &= expectTrue(!all.has_value(), "getAllTestNames returns nullopt when problem config missing");

    all = getAllTestNames("problem_empty_config");
    success &= expectTrue(!all.has_value(), "getAllTestNames returns nullopt when problem config empty");

    all = getAllTestNames("unexistent_problem");
    success &= expectTrue(!all.has_value(), "getAllTestNames returns nullopt when problem does not exist");

    return success;
}
