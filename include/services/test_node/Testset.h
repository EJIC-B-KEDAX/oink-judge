#pragma once

#include "Test.hpp"
#include "VerdictBuilder.hpp"

namespace oink_judge::services::test_node {

class Testset : public Test {
public:
    Testset(ProblemBuilder *problem_builder, const std::string &problem_id, const std::string &testset_name);

    std::shared_ptr<Verdict> run(const std::string &submission_id, const std::vector<std::string> &boxes, json additional_params) override;
    std::shared_ptr<Verdict> skip(const std::string &submission_id) override;
    size_t boxes_required() const override;
    const std::string &get_name() const override;

    constexpr static auto REGISTERED_NAME = "testset";

private:
    std::string _name;
    double _time_limit;
    double _memory_limit;
    double _real_time_limit;
    std::vector<std::shared_ptr<Test>> _tests;
    std::unique_ptr<VerdictBuilder> _verdict_builder;
};

} // namespace oink_judge::services::test_node
