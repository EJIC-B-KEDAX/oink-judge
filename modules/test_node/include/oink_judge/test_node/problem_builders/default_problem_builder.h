#pragma once
#include "oink_judge/test_node/problem_builder.hpp"
#include "oink_judge/test_node/problem_builders/enable_get_test_by_name_impl.h"

namespace oink_judge::test_node {

class DefaultProblemBuilder : public ProblemBuilder, public EnableGetTestByNameImpl {
  public:
    DefaultProblemBuilder(std::string problem_id);
    auto build() -> std::shared_ptr<Test> override;

    constexpr static auto REGISTERED_NAME = "DefaultProblemBuilder";

  private:
    std::string problem_id_;
};

} // namespace oink_judge::test_node
