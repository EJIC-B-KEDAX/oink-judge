#include "config/problem_config_utils.h"
#include "config/Config.h"
#include <iostream>

namespace oink_judge::problem_config {

using Config = config::Config;

pugi::xml_node get_problem_config(const std::string &problem_id) {
    std::cout << "Need problem config" << std::endl;
    pugi::xml_document problem_config;
    std::string problem_config_path = Config::config().at("directories").at("problems").get<std::string>() + "/" + problem_id + "/problem.xml";
    pugi::xml_parse_result result = problem_config.load_file(problem_config_path.c_str());
    if (!result) {
        throw std::runtime_error("Failed to load problem config: " + problem_config_path);
    }

    return problem_config.child("problem");
}

std::string get_problem_builder_name(const std::string &problem_id) {
    pugi::xml_node problem = get_problem_config(problem_id);
    return problem.child("problem_builder").attribute("type").as_string();
}

pugi::xml_node get_test_config(const std::string &problem_id, const std::string &test_name) {
    pugi::xml_node problem = get_problem_config(problem_id);

    for (pugi::xml_node test_node : problem.child("tests").children("test")) {
        if (test_node.attribute("name").as_string() == test_name) {
            return test_node;
        }
    }

    throw std::runtime_error("Test not found: " + test_name);
}

std::vector<std::string> get_all_test_names(const std::string &problem_id) {
    pugi::xml_node problem = get_problem_config(problem_id);
    std::vector<std::string> test_names;

    for (pugi::xml_node test_node : problem.child("tests").children("test")) {
        std::string test_name = test_node.attribute("name").as_string();
        if (!test_name.empty()) {
            test_names.push_back(test_name);
        }
    }

    return test_names;
}

} // namespace oink_judge::problem_config
