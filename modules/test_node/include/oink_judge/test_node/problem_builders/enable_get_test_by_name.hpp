#pragma once
#include "oink_judge/test_node/test.hpp"

namespace oink_judge::test_node {

class EnableGetTestByName {
  public:
    EnableGetTestByName(const EnableGetTestByName&) = default;
    auto operator=(const EnableGetTestByName&) -> EnableGetTestByName& = default;
    EnableGetTestByName(EnableGetTestByName&&) = default;
    auto operator=(EnableGetTestByName&&) -> EnableGetTestByName& = default;
    virtual ~EnableGetTestByName() = default;

    virtual auto getTestByName(const std::string& name) -> std::shared_ptr<Test> = 0;

  protected:
    EnableGetTestByName() = default;
};

} // namespace oink_judge::test_node
