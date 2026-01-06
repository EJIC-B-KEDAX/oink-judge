#include "services/data_sender/problem_package_converter/PolygonConverter.h"
#include "config/problem_config_utils.h"

namespace oink_judge::services::data_sender::problem_package_converter {

namespace {

[[maybe_unused]] bool registered = []() -> bool {
    PackageConverterFactory::instance().register_type(PolygonConverter::REGISTERED_NAME,
    [](const std::string &params) -> std::shared_ptr<PackageConverter> {
        return std::make_shared<PolygonConverter>();
    });

    return true;
}();

std::string get_problem_id_from_path(const std::string &path_to_package) {
    std::string problem_id;
    for (int i = static_cast<int>(path_to_package.size()) - 1; i >= 0; --i) {
        if (path_to_package[i] == '/' || path_to_package[i] == '\\') {
            break;
        }
        problem_id.push_back(path_to_package[i]);
    }
    std::reverse(problem_id.begin(), problem_id.end());

    return problem_id;
}

} // namespace

PolygonConverter::PolygonConverter() = default;

void PolygonConverter::convert_package(const std::string &path_to_package) {
    convert_icpc_problem_package(path_to_package);
}

void PolygonConverter::convert_icpc_problem_package(const std::string &path_to_package) {
    std::string problem_id = get_problem_id_from_path(path_to_package);
    pugi::xml_node problem_config = oink_judge::problem_config::get_problem_config(problem_id);

    problem_config.append_attribute("type").set_value("open_problem");

    problem_config.append_child("problem_builder").append_attribute("type").set_value("DefaultProblemBuilder");
    problem_config.append_child("submission_manager").append_attribute("type").set_value("BasicProblemSubmissionManager");

    pugi::xml_node tests_node = problem_config.append_child("tests");
    pugi::xml_node judging_node = problem_config.child("judging");

    for (pugi::xml_node testset : judging_node.children("testset")) {
        size_t tests_count = testset.child("test-count").text().as_ullong();
        std::string testset_name = testset.attribute("name").as_string();
        unsigned long long time_limit = testset.child("time-limit").text().as_ullong();
        unsigned long long memory_limit = testset.child("memory-limit").text().as_ullong();
        unsigned long long real_time_limit = std::max<unsigned long long>(2 * time_limit, 5000);
        std::string verdict_builder_type = "min";

        for (size_t test_num = 1; test_num <= tests_count; test_num++) {
            pugi::xml_node cur_test = tests_node.append_child("test");
            cur_test.append_attribute("type").set_value("single");
            std::string test_name = testset_name + std::to_string(test_num);
            cur_test.append_attribute("name").set_value(test_name.c_str());
        }

        pugi::xml_node testset_node = tests_node.append_child("test");
        testset_node.append_attribute("type").set_value("testset");
        testset_node.append_attribute("name").set_value(testset_name.c_str());
        testset_node.append_child("time_limit").text() = time_limit;
        testset_node.append_child("memory_limit").text() = memory_limit;
        testset_node.append_child("real_time_limit").text() = real_time_limit;
        testset_node.append_child("verdict_builder").append_attribute("type").set_value(verdict_builder_type.c_str());

        for (size_t test_num = 1; test_num <= tests_count; test_num++) {
            pugi::xml_node cur_test = testset_node.append_child("test");
            std::string test_name = testset_name + std::to_string(test_num);
            cur_test.append_attribute("name").set_value(test_name.c_str());
        }
    }

    pugi::xml_node compilation_node = tests_node.append_child("test");
    compilation_node.append_attribute("type").set_value("compilation");
    compilation_node.append_attribute("name").set_value("compilation");
    compilation_node.append_child("test").append_attribute("name").set_value("tests");

    pugi::xml_node sync_result_node = tests_node.append_child("test");
    sync_result_node.append_attribute("type").set_value("sync_result");
    sync_result_node.append_attribute("name").set_value("main");
    sync_result_node.append_child("test").append_attribute("name").set_value("compilation");

    pugi::xml_document config_document;
    config_document.append_child("problem") = problem_config;
    if (!config_document.save_file((path_to_package + "/problem.xml").c_str())) {
        throw std::runtime_error("Can not save config file");
    }
}

} // namespace oink_judge::services::data_sender::problem_package_converter
