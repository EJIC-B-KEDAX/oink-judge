#include "services/test_node/CompilationTest.h"
#include "services/test_node/problem_utils.h"
#include "services/test_node/enable_get_test_by_name.hpp"
#include "services/test_node/ProblemBuilder.hpp"
#include "services/test_node/TableSubmissions.h"
#include "config/Config.h"
#include <iostream>

namespace oink_judge::services::test_node {

using Config = config::Config;

namespace {

[[maybe_unused]] bool registered = []() -> bool {
    TestFactory::instance().register_type(CompilationTest::REGISTERED_NAME,
        [](ProblemBuilder *problem_builder, const std::string &problem_id, const std::string &name) -> std::shared_ptr<Test> {
        return std::make_shared<CompilationTest>(problem_builder, problem_id, name);
    });

    return true;
}();

} // namespace

CompilationTest::CompilationTest(ProblemBuilder *problem_builder, const std::string &problem_id, const std::string &name)
    : _name(name), _problem_id(problem_id) {
    pugi::xml_node testset_config = get_test_config(problem_id, name);

    enable_get_test_by_name *problem_builder_with_getter = dynamic_cast<enable_get_test_by_name*>(problem_builder);

    pugi::xml_node test_node = testset_config.child("test");

    std::string test_name = test_node.attribute("name").as_string();
    _test = problem_builder_with_getter->get_test_by_name(test_name);
    if (!_test) {
        throw std::runtime_error("Test not found: " + test_name);
    }
}

std::shared_ptr<Verdict> CompilationTest::run(const std::string &submission_id, const std::vector<std::string> &boxes, json additional_params) {
    std::cout << "Compilation testing " << get_name() << std::endl;
    if (boxes.size() < boxes_required()) {
        throw std::runtime_error("Not enough boxes provided");
    }

    int rc = std::system((Config::config().at("directories").at("scripts").get<std::string>() + "/prepare_for_testing.sh " + boxes[0] + " " + boxes[1] + 
        " " + _problem_id + " " + submission_id + " " + TableSubmissions::instance().language_of_submission(submission_id)).c_str());

    if (rc != 0) {
        auto verdict = std::make_shared<DefaultVerdict>(_name);
        DefaultVerdict::VerdictInfo info;
        info.type = VerdictType(VerdictType::WRONG, "Compilation Error", "CE");
        info.score = 0.0;
        info.time_used = 0.0;
        info.memory_used = 0.0;
        info.real_time_used = 0.0;
        verdict->set_info(info);

        return verdict;
    }

    return _test->run(submission_id, boxes, additional_params);
}

std::shared_ptr<Verdict> CompilationTest::skip(const std::string &submission_id) {
    return _test->skip(submission_id);
}

size_t CompilationTest::boxes_required() const {
    return _test->boxes_required();
}

const std::string &CompilationTest::get_name() const {
    return _name;
}

} // namespace oink_judge::services::test_node
