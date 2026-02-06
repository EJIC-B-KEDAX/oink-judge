#pragma once
#include "oink_judge/test_node/problem_builders/enable_get_test_by_name.hpp"

namespace oink_judge::test_node {

class EnableGetTestByNameImpl : public EnableGetTestByName {
  public:
    EnableGetTestByNameImpl();

    auto getTestByName(const std::string& name) -> std::shared_ptr<Test> override;

  protected:
    auto addTest(const std::shared_ptr<Test>& test) -> void;

  private:
    std::map<std::string, std::shared_ptr<Test>> tests_;
};

} // namespace oink_judge::test_node
