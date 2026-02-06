#include "oink_judge/test_node/tests/testset.h"

#include "oink_judge/test_node/problem_builder.hpp"
#include "oink_judge/test_node/problem_builders/enable_get_test_by_name.hpp"

#include <oink_judge/config/problem_config_utils.h>

namespace oink_judge::test_node {

namespace {

[[maybe_unused]] const bool REGISTERED = []() -> bool {
    TestFactory::instance().registerType(Testset::REGISTERED_NAME,
                                         [](ProblemBuilder* problem_builder, const std::string& problem_id,
                                            const std::string& testset_name) -> std::shared_ptr<Test> {
                                             return std::make_shared<Testset>(problem_builder, problem_id, testset_name);
                                         });

    return true;
}();

} // namespace

Testset::Testset(ProblemBuilder* problem_builder, const std::string& problem_id, std::string testset_name)
    : name_(std::move(testset_name)) {
    pugi::xml_node testset_config = problem_config::getTestConfig(problem_id, name_).value_or(pugi::xml_node{});
    time_limit_ = testset_config.child("time_limit").text().as_double() / 1000;                                  // NOLINT
    memory_limit_ = testset_config.child("memory_limit").text().as_double() / 1024;                              // NOLINT
    real_time_limit_ = testset_config.child("real_time_limit").text().as_double(std::max(5.0, 2 * time_limit_)); // NOLINT

    std::string verdict_builder_type = testset_config.child("verdict_builder").attribute("type").as_string();
    verdict_builder_ = VerdictBuilderFactory::instance().create(verdict_builder_type, name_);

    auto* problem_builder_with_getter = dynamic_cast<EnableGetTestByName*>(problem_builder);

    for (pugi::xml_node test_node : testset_config.children("test")) {
        std::string test_name = test_node.attribute("name").as_string();
        std::shared_ptr<Test> cur_test = problem_builder_with_getter->getTestByName(test_name);
        if (!cur_test) {
            throw std::runtime_error("Test not found: " + test_name);
        }
        tests_.push_back(cur_test);
    }
}

std::shared_ptr<Verdict> Testset::run(const std::string& submission_id, const std::vector<std::string>& boxes,
                                      json additional_params) {
    if (boxes.size() < boxesRequired()) {
        throw std::runtime_error("Not enough boxes provided");
    }

    additional_params["time_limit"] = time_limit_;
    additional_params["memory_limit"] = memory_limit_;
    additional_params["real_time_limit"] = real_time_limit_;

    verdict_builder_->clear();

    for (size_t i = 0; i < tests_.size(); ++i) {
        std::shared_ptr<Verdict> test_verdict = nullptr;

        if (!verdict_builder_->canScoreChange()) {
            test_verdict = tests_[i]->skip(submission_id);
        } else {
            test_verdict = tests_[i]->run(submission_id, boxes, additional_params);
        }
        verdict_builder_->addVerdict(std::dynamic_pointer_cast<DefaultVerdict>(test_verdict));
    }

    return verdict_builder_->finalize();
}

std::shared_ptr<Verdict> Testset::skip(const std::string& submission_id) {
    verdict_builder_->clear();

    for (size_t i = 0; i < tests_.size(); ++i) {
        std::shared_ptr<Verdict> test_verdict = tests_[i]->skip(submission_id);
        verdict_builder_->addVerdict(std::dynamic_pointer_cast<DefaultVerdict>(test_verdict));
    }

    return verdict_builder_->finalize();
}

size_t Testset::boxesRequired() const {
    size_t max_boxes = 0;
    for (const auto& test : tests_) {
        max_boxes = std::max(max_boxes, test->boxesRequired());
    }
    return max_boxes;
}

const std::string& Testset::getName() const { return name_; }

} // namespace oink_judge::test_node
