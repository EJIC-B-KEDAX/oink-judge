#include "services/test_node/tests/CompilationTest.h"

#include "config/Config.h"
#include "config/problem_config_utils.h"
#include "database/TableSubmissions.h"
#include "services/test_node/ProblemBuilder.hpp"
#include "services/test_node/problem_builders/enable_get_test_by_name.hpp"

#include <iostream>

namespace oink_judge::services::test_node {

using Config = config::Config;
using TableSubmissions = database::TableSubmissions;

namespace {

[[maybe_unused]] bool registered = []() -> bool {
    TestFactory::instance().register_type(
        CompilationTest::REGISTERED_NAME,
        [](ProblemBuilder* problem_builder, const std::string& problem_id, const std::string& name) -> std::shared_ptr<Test> {
            return std::make_shared<CompilationTest>(problem_builder, problem_id, name);
        });

    return true;
}();

} // namespace

CompilationTest::CompilationTest(ProblemBuilder* problem_builder, const std::string& problem_id, const std::string& name)
    : _name(name), _problem_id(problem_id) {
    pugi::xml_node testset_config = problem_config::getTestConfig(problem_id, name);

    enable_get_test_by_name* problem_builder_with_getter = dynamic_cast<enable_get_test_by_name*>(problem_builder);

    pugi::xml_node test_node = testset_config.child("test");

    std::string test_name = test_node.attribute("name").as_string();
    _test = problem_builder_with_getter->get_test_by_name(test_name);
    if (!_test) {
        throw std::runtime_error("Test not found: " + test_name);
    }
}

std::shared_ptr<Verdict> CompilationTest::run(const std::string& submission_id, const std::vector<std::string>& boxes,
                                              json additional_params) {
    std::cout << "Compilation testing " << get_name() << std::endl;
    if (boxes.size() < boxes_required()) {
        throw std::runtime_error("Not enough boxes provided");
    }

    std::string scripts_dir = Config::config().at("directories").at("scripts").get<std::string>();
    std::string submissions_dir = Config::config().at("directories").at("submissions").get<std::string>();
    std::string problems_dir = Config::config().at("directories").at("problems").get<std::string>();

    std::string language = TableSubmissions::instance().language_of_submission(submission_id);

    std::string compilation_script = scripts_dir + "/compilation/" + language + ".sh";
    std::string output_executale_name = "/var/local/lib/isolate/" + boxes[0] + "/box/solution";
    std::string error_file_name = submissions_dir + "/" + submission_id + "/compilation_error.txt";
    std::string source_file_name = submissions_dir + "/" + submission_id + "/source.cpp";

    int rc_prepare = std::system((Config::config().at("directories").at("scripts").get<std::string>() +
                                  "/prepare_for_testing.sh " + boxes[0] + " " + boxes[1] + " " + _problem_id)
                                     .c_str());

    if (rc_prepare != 0) {
        auto verdict = std::make_shared<DefaultVerdict>(_name);
        DefaultVerdict::VerdictInfo info;
        info.type = VerdictType::FAILED;
        info.score = 0.0;
        info.time_used = 0.0;
        info.memory_used = 0.0;
        info.real_time_used = 0.0;
        verdict->set_info(info);

        return verdict;
    }

    int rc_compilation =
        std::system((compilation_script + " " + output_executale_name + " " + error_file_name + " " + source_file_name).c_str());

    if (rc_compilation != 0) {
        auto verdict = std::make_shared<DefaultVerdict>(_name);
        DefaultVerdict::VerdictInfo info;
        info.type = VerdictType::COMPILATION_ERROR;
        info.score = 0.0;
        info.time_used = 0.0;
        info.memory_used = 0.0;
        info.real_time_used = 0.0;
        verdict->set_info(info);

        return verdict;
    }

    int rc_copy_checker = std::system(
        ("cp " + problems_dir + "/" + _problem_id + "/checker /var/local/lib/isolate/" + boxes[1] + "/box/checker").c_str());

    if (rc_copy_checker != 0) {
        auto verdict = std::make_shared<DefaultVerdict>(_name);
        DefaultVerdict::VerdictInfo info;
        info.type = VerdictType::FAILED;
        info.score = 0.0;
        info.time_used = 0.0;
        info.memory_used = 0.0;
        info.real_time_used = 0.0;
        verdict->set_info(info);

        return verdict;
    }

    std::shared_ptr<Verdict> result = _test->run(submission_id, boxes, additional_params);

    int rc_clear = std::system(
        (Config::config().at("directories").at("scripts").get<std::string>() + "/end_testing.sh " + boxes[0] + " " + boxes[1])
            .c_str());

    if (rc_clear != 0) {
        auto verdict = std::make_shared<DefaultVerdict>(_name);
        DefaultVerdict::VerdictInfo info;
        info.type = VerdictType::FAILED;
        info.score = 0.0;
        info.time_used = 0.0;
        info.memory_used = 0.0;
        info.real_time_used = 0.0;
        verdict->set_info(info);

        return verdict;
    }

    return result;
}

std::shared_ptr<Verdict> CompilationTest::skip(const std::string& submission_id) { return _test->skip(submission_id); }

size_t CompilationTest::boxes_required() const { return _test->boxes_required(); }

const std::string& CompilationTest::get_name() const { return _name; }

} // namespace oink_judge::services::test_node
