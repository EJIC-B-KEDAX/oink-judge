#include "oink_judge/config/problem_config_utils.h"

#include "oink_judge/config/common_utils.h"

#include <oink_judge/logger/logger.h>
#include <oink_judge/utils/filesystem.h>

#include <cstring>
#include <filesystem>
#include <map>
#include <stdexcept>

namespace oink_judge::problem_config {

namespace fs = std::filesystem;
using config::requireHasValue;

auto getFullProblemConfig(const std::string& problem_id) -> const pugi::xml_document& {
    static std::map<std::string, pugi::xml_document> problem_config_cache;

    logger::logMessage("problem_config", "Fetching full problem config for problem ID: " + problem_id, logger::LogType::DEBUG, 3);
    fs::path problems_dir = requireHasValue(config::getDirectoryPath("problems"));

    std::filesystem::path problem_config_path = problems_dir / problem_id / "problem.xml";
    pugi::xml_parse_result result = problem_config_cache[problem_id].load_file(problem_config_path.c_str());
    if (!result) {
        throw std::runtime_error("Failed to load problem config from " + problem_config_path.string() + ": " +
                                 result.description());
    }

    return problem_config_cache[problem_id];
}

auto getProblemConfig(const std::string& problem_id) -> std::optional<pugi::xml_node> {
    try {
        const auto& full_config = getFullProblemConfig(problem_id);
        pugi::xml_node problem = full_config.child("problem");
        if (!problem) {
            return std::nullopt;
        }
        return problem;
    } catch (const std::runtime_error& e) {
        logger::logError("problem_config", "Error getting problem config for problem '" + problem_id + "': " + e.what());
        return std::nullopt;
    }
}

auto getProblemBuilderName(const std::string& problem_id) -> std::optional<std::string> {
    std::optional<pugi::xml_node> problem_opt = getProblemConfig(problem_id);
    if (!problem_opt.has_value()) {
        return std::nullopt;
    }
    pugi::xml_node problem = problem_opt.value();
    std::string builder_type = problem.child("problem_builder").attribute("type").as_string();
    if (builder_type.empty()) {
        return std::nullopt;
    }
    return builder_type;
}

auto getTestConfig(const std::string& problem_id, const std::string& test_name) -> std::optional<pugi::xml_node> {
    std::optional<pugi::xml_node> problem_opt = getProblemConfig(problem_id);
    if (!problem_opt.has_value()) {
        return std::nullopt;
    }
    pugi::xml_node problem = problem_opt.value();

    for (pugi::xml_node test_node : problem.child("tests").children("test")) {
        if (test_node.attribute("name").as_string() == test_name) {
            return test_node;
        }
    }

    return std::nullopt;
}

auto getAllTestNames(const std::string& problem_id) -> std::optional<std::vector<std::string>> {
    std::optional<pugi::xml_node> problem_opt = getProblemConfig(problem_id);
    if (!problem_opt.has_value()) {
        return std::nullopt;
    }
    pugi::xml_node problem = problem_opt.value();
    std::vector<std::string> test_names;

    pugi::xml_node tests_node = problem.child("tests");
    if (!tests_node) {
        return std::nullopt;
    }

    for (pugi::xml_node test_node : tests_node.children("test")) {
        std::string test_name = test_node.attribute("name").as_string();
        if (!test_name.empty()) {
            test_names.push_back(test_name);
        }
    }

    return test_names;
}

auto getPathToProblemStatements(const std::string& problem_id, const std::string& language, const std::string& type)
    -> std::optional<fs::path> {
    std::optional<pugi::xml_node> problem_opt = getProblemConfig(problem_id);
    if (!problem_opt) {
        return std::nullopt;
    }
    std::optional<fs::path> problem_dir_opt = config::getDirectoryPath("problems");
    if (!problem_dir_opt) {
        return std::nullopt;
    }
    pugi::xml_node problem_config = *problem_opt;
    const fs::path& problems_dir = *problem_dir_opt;

    pugi::xml_node statements = problem_config.child("statements");
    for (auto statement : statements.children("statement")) {
        if (statement.attribute("language").as_string() == language && statement.attribute("type").as_string() == type) {
            if (std::strlen(statement.attribute("path").as_string()) != 0) {
                fs::path result = problems_dir / problem_id / statement.attribute("path").as_string();
                if (fs::is_regular_file(result)) {
                    return result;
                }
            }
        }
    }
    return std::nullopt;
}

auto getProblemStatements(const std::string& problem_id, const std::string& language, const std::string& type)
    -> std::optional<std::string> {
    std::optional<fs::path> statements_path_opt = getPathToProblemStatements(problem_id, language, type);
    if (!statements_path_opt) {
        return std::nullopt;
    }
    return utils::filesystem::loadFile(*statements_path_opt);
}

} // namespace oink_judge::problem_config
