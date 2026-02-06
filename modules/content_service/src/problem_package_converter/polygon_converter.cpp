#include "oink_judge/content_service/problem_package_converter/polygon_converter.h"

#include <algorithm>
#include <oink_judge/config/problem_config_utils.h>

namespace oink_judge::content_service::problem_package_converter {

namespace {

[[maybe_unused]] const bool REGISTERED = []() -> bool {
    PackageConverterFactory::instance().registerType(
        PolygonConverter::REGISTERED_NAME,
        [](const std::string& params) -> std::shared_ptr<PackageConverter> { return std::make_shared<PolygonConverter>(); });

    return true;
}();

auto getProblemIdFromPath(const std::string& path_to_package) -> std::string {
    std::string problem_id;
    for (int i = static_cast<int>(path_to_package.size()) - 1; i >= 0; --i) {
        if ((path_to_package[i] == '/' || path_to_package[i] == '\\') && !problem_id.empty()) {
            break;
        }
        problem_id.push_back(path_to_package[i]);
    }
    std::ranges::reverse(problem_id);

    return problem_id;
}

} // namespace

PolygonConverter::PolygonConverter() = default;

auto PolygonConverter::convertPackage(const std::string& path_to_package) -> void { convertICPCProblemPackage(path_to_package); }

auto PolygonConverter::convertICPCProblemPackage(const std::string& path_to_package) -> void { // NOLINT
    std::string problem_id = getProblemIdFromPath(path_to_package);
    auto problem_config_opt = oink_judge::problem_config::getProblemConfig(problem_id);
    if (!problem_config_opt) {
        throw std::runtime_error("Can not find problem config for problem " + problem_id);
    }
    pugi::xml_node problem_config = *problem_config_opt;

    std::string path_to_checker =
        path_to_package + problem_config.child("assets").child("checker").child("source").attribute("path").as_string();

    int compile_checker_rc =
        std::system(("g++ -o " + path_to_package + "/checker " + path_to_checker + " -O2 -std=c++23").c_str());
    if (compile_checker_rc != 0) {
        throw std::runtime_error("Can not compile checker");
    }

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
        unsigned long long real_time_limit = std::max<unsigned long long>(2 * time_limit, 5000); // NOLINT
        std::string verdict_builder_type = "min";

        for (size_t test_num = 1; test_num <= tests_count; test_num++) {
            pugi::xml_node cur_test = tests_node.append_child("test");
            cur_test.append_attribute("type").set_value("single");
            std::string test_name = testset_name + std::to_string(test_num);
            if (testset_name == "tests") {
                test_name = std::to_string(test_num);
            }
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
            if (testset_name == "tests") {
                test_name = std::to_string(test_num);
            }
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
    config_document.append_copy(problem_config);
    if (!config_document.save_file((path_to_package + "/problem.xml").c_str())) {
        throw std::runtime_error("Can not save config file");
    }
}

} // namespace oink_judge::content_service::problem_package_converter
