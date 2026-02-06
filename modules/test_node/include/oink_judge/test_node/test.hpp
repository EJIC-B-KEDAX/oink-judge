#pragma once
#include "oink_judge/test_node/verdict.hpp"

#include <oink_judge/factory/type_factory.hpp>
#include <string>
#include <vector>

namespace oink_judge::test_node {

class ProblemBuilder;

class Test {
  public:
    Test(const Test&) = delete;
    auto operator=(const Test&) -> Test& = delete;
    Test(Test&&) = delete;
    auto operator=(Test&&) -> Test& = delete;
    virtual ~Test() = default;

    virtual auto run(const std::string& submission_id, const std::vector<std::string>& boxes, json additional_params)
        -> std::shared_ptr<Verdict> = 0;
    virtual auto skip(const std::string& submission_id) -> std::shared_ptr<Verdict> = 0;
    [[nodiscard]] virtual auto boxesRequired() const -> size_t = 0;
    [[nodiscard]] virtual auto getName() const -> const std::string& = 0;

  protected:
    Test() = default;
};

using TestFactory = factory::TypeFactory<std::shared_ptr<Test>, ProblemBuilder*, const std::string&, const std::string&>;

} // namespace oink_judge::test_node
