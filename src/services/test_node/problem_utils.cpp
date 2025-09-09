#include "services/test_node/problem_utils.h"
#include "config/Config.h"

namespace oink_judge::services::test_node {

using Config = config::Config;

pugi::xml_node get_problem_config(const std::string &problem_id) {
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

pugi::xml_node get_test_config(const std::string &problem_id, const std::string &testset_name) {
    pugi::xml_node problem = get_problem_config(problem_id);

    for (pugi::xml_node testset_node : problem.child("tests").children("test")) {
        if (testset_node.attribute("name").as_string() == testset_name) {
            return testset_node;
        }
    }

    throw std::runtime_error("Testset not found: " + testset_name);
}

std::vector<std::string> get_all_test_names(const std::string &problem_id) {
    pugi::xml_node problem = get_problem_config(problem_id);
    std::vector<std::string> test_names;

    for (pugi::xml_node testset_node : problem.child("tests").children("test")) {
        std::string testset_name = testset_node.attribute("name").as_string();
        if (!testset_name.empty()) {
            test_names.push_back(testset_name);
        }
    }

    return test_names;
}

} // namespace oink_judge::services::test_node
