#pragma once
#include "oink_judge/test_node/test.hpp"

namespace oink_judge::test_node {

class CompilationTest : public Test {
  public:
    CompilationTest(ProblemBuilder* problem_builder, std::string problem_id, std::string name);

    auto run(const std::string& submission_id, const std::vector<std::string>& boxes, json additional_params)
        -> std::shared_ptr<Verdict> override;
    auto skip(const std::string& submission_id) -> std::shared_ptr<Verdict> override;
    [[nodiscard]] auto boxesRequired() const -> size_t override;
    [[nodiscard]] auto getName() const -> const std::string& override;

    constexpr static auto REGISTERED_NAME = "compilation";

  private:
    std::string name_;
    std::string problem_id_;
    std::shared_ptr<Test> test_;
};

} // namespace oink_judge::test_node
