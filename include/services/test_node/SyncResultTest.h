#pragma once

#include "Test.hpp"
#include "VerdictBuilder.hpp"

namespace oink_judge::services::test_node {

class SyncResultTest : public Test {
public:
    SyncResultTest(ProblemBuilder *problem_builder, const std::string &problem_id, const std::string &name);

    std::shared_ptr<Verdict> run(const std::string &submission_id, const std::vector<std::string> &boxes, json additional_params) override;
    std::shared_ptr<Verdict> skip(const std::string &submission_id) override;
    size_t boxes_required() const override;
    const std::string &get_name() const override;

    constexpr static auto REGISTERED_NAME = "sync_result";

private:
    std::string _name;
    std::string _problem_id;
    std::shared_ptr<Test> _test;
};

} // namespace oink_judge::services::test_node
