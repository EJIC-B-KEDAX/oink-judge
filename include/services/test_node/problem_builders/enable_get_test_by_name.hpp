#pragma once
#include "services/test_node/Test.hpp"

namespace oink_judge::services::test_node {

class enable_get_test_by_name {
public:
    virtual std::shared_ptr<Test> get_test_by_name(const std::string &name) = 0;
};

} // namespace oink_judge::services::test_node
