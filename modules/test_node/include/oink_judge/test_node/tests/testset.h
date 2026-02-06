#pragma once
#include "oink_judge/test_node/test.hpp"
#include "oink_judge/test_node/verdict_builder.hpp"

namespace oink_judge::test_node {

class Testset : public Test {
  public:
    Testset(ProblemBuilder* problem_builder, const std::string& problem_id, std::string testset_name);

    auto run(const std::string& submission_id, const std::vector<std::string>& boxes, json additional_params)
        -> std::shared_ptr<Verdict> override;
    auto skip(const std::string& submission_id) -> std::shared_ptr<Verdict> override;
    [[nodiscard]] auto boxesRequired() const -> size_t override;
    [[nodiscard]] auto getName() const -> const std::string& override;

    constexpr static auto REGISTERED_NAME = "testset";

  private:
    std::string name_;
    double time_limit_;
    double memory_limit_;
    double real_time_limit_;
    std::vector<std::shared_ptr<Test>> tests_;
    std::unique_ptr<VerdictBuilder> verdict_builder_;
};

} // namespace oink_judge::test_node
