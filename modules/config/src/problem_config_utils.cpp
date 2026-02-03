#include "oink_judge/config/problem_config_utils.h"

#include "oink_judge/config/config.h"

#include <oink_judge/logger/logger.h>
#include <stdexcept>

namespace oink_judge::problem_config {

using Config = config::Config;

auto getFullProblemConfig(const std::string& problem_id) -> pugi::xml_document& {
    static std::map<std::string, pugi::xml_document> problem_config_cache;

    logger::logMessage("problem_config", 3, "Fetching full problem config for problem ID: " + problem_id, logger::LogType::DEBUG);
    std::optional<std::filesystem::path> problems_dir = config::getDirectoryPath("problems");
    if (!problems_dir.has_value()) {
        throw std::runtime_error("Directory 'problems' is not configured");
    }
    std::filesystem::path problem_config_path = problems_dir.value() / problem_id / "problem.xml";
    pugi::xml_parse_result result = problem_config_cache[problem_id].load_file(problem_config_path.c_str());
    if (!result) {
        throw std::runtime_error("Failed to load problem config from " + problem_config_path.string() + ": " +
                                 result.description());
    }

    return problem_config_cache[problem_id];
}

auto getProblemConfig(const std::string& problem_id) -> std::optional<pugi::xml_node> {
    try {
        auto& full_config = getFullProblemConfig(problem_id);
        pugi::xml_node problem = full_config.child("problem");
        if (!problem) {
            return std::nullopt;
        }
        return problem;
    } catch (const std::runtime_error& e) {
        logger::logMessage("problem_config", 1, "Error getting problem config for problem '" + problem_id + "': " + e.what(),
                           logger::LogType::ERROR);
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

    return std::move(test_names);
}

} // namespace oink_judge::problem_config
