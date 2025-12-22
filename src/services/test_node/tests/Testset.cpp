#include "services/test_node/tests/Testset.h"
#include "config/problem_config_utils.h"
#include "services/test_node/problem_builders/enable_get_test_by_name.hpp"
#include "services/test_node/ProblemBuilder.hpp"
#include <iostream> 

namespace oink_judge::services::test_node {

namespace {

[[maybe_unused]] bool registered = []() -> bool {
    TestFactory::instance().register_type(Testset::REGISTERED_NAME,
        [](ProblemBuilder *problem_builder, const std::string &problem_id, const std::string &testset_name) -> std::shared_ptr<Test> {
        return std::make_shared<Testset>(problem_builder, problem_id, testset_name);
    });

    return true;
}();

} // namespace

Testset::Testset(ProblemBuilder *problem_builder, const std::string &problem_id, const std::string &testset_name)
    : _name(testset_name) {
    pugi::xml_node testset_config = problem_config::get_test_config(problem_id, testset_name);
    _time_limit = testset_config.child("time_limit").text().as_double() / 1000;
    _memory_limit = testset_config.child("memory_limit").text().as_double() / 1024;
    _real_time_limit = testset_config.child("real_time_limit").text().as_double(std::max(5.0, 2 * _time_limit));

    std::string verdict_builder_type = testset_config.child("verdict_builder").attribute("type").as_string();
    _verdict_builder = VerdictBuilderFactory::instance().create(verdict_builder_type, _name);

    enable_get_test_by_name *problem_builder_with_getter = dynamic_cast<enable_get_test_by_name*>(problem_builder);

    for (pugi::xml_node test_node : testset_config.children("test")) {
        std::cout << test_node.attribute("name").as_string() << std::endl;
        std::string test_name = test_node.attribute("name").as_string();
        std::shared_ptr<Test> cur_test = problem_builder_with_getter->get_test_by_name(test_name);
        if (!cur_test) {
            throw std::runtime_error("Test not found: " + test_name);
        }
        _tests.push_back(cur_test);
    }

    std::cout << "SIZE: " << _tests.size() << std::endl;
}

std::shared_ptr<Verdict> Testset::run(const std::string &submission_id, const std::vector<std::string> &boxes, json additional_params) {
    std::cout << "Testset testing " << get_name() << std::endl;
    if (boxes.size() < boxes_required()) {
        throw std::runtime_error("Not enough boxes provided");
    }

    additional_params["time_limit"] = _time_limit;
    additional_params["memory_limit"] = _memory_limit;
    additional_params["real_time_limit"] = _real_time_limit;

    _verdict_builder->clear();

    for (size_t i = 0; i < _tests.size(); ++i) {
        std::shared_ptr<Verdict> test_verdict = nullptr;

        if (!_verdict_builder->can_score_change()) test_verdict = _tests[i]->skip(submission_id);
        else test_verdict = _tests[i]->run(submission_id, boxes, additional_params);

        _verdict_builder->add_verdict(std::dynamic_pointer_cast<DefaultVerdict>(test_verdict));
    }

    return _verdict_builder->finalize();
}

std::shared_ptr<Verdict> Testset::skip(const std::string &submission_id) {
    _verdict_builder->clear();

    for (size_t i = 0; i < _tests.size(); ++i) {
        std::shared_ptr<Verdict> test_verdict = _tests[i]->skip(submission_id);
        _verdict_builder->add_verdict(std::dynamic_pointer_cast<DefaultVerdict>(test_verdict));
    }

    return _verdict_builder->finalize();
}

size_t Testset::boxes_required() const {
    size_t max_boxes = 0;
    for (const auto &test : _tests) {
        max_boxes = std::max(max_boxes, test->boxes_required());
    }
    return max_boxes;
}

const std::string &Testset::get_name() const {
    return _name;
}

} // namespace oink_judge::services::test_node
