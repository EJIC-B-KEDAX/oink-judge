#pragma once
#include "oink_judge/test_node/test.hpp"

#include <oink_judge/factory/type_factory.hpp>

namespace oink_judge::test_node {

class ProblemBuilder {
  public:
    ProblemBuilder(const ProblemBuilder&) = delete;
    auto operator=(const ProblemBuilder&) -> ProblemBuilder& = delete;
    ProblemBuilder(ProblemBuilder&&) = delete;
    auto operator=(ProblemBuilder&&) -> ProblemBuilder& = delete;
    virtual ~ProblemBuilder() = default;

    virtual auto build() -> std::shared_ptr<Test> = 0;

  protected:
    ProblemBuilder() = default;
};

using ProblemBuilderFactory = factory::TypeFactory<std::shared_ptr<ProblemBuilder>, const std::string&>;

} // namespace oink_judge::test_node
