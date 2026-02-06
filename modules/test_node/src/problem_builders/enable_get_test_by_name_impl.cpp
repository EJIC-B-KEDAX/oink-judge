#include "oink_judge/test_node/problem_builders/enable_get_test_by_name_impl.h"

namespace oink_judge::test_node {

EnableGetTestByNameImpl::EnableGetTestByNameImpl() = default;

auto EnableGetTestByNameImpl::getTestByName(const std::string& name) -> std::shared_ptr<Test> {
    auto it = tests_.find(name);
    if (it != tests_.end()) {
        return it->second;
    }
    return nullptr;
}

void EnableGetTestByNameImpl::addTest(const std::shared_ptr<Test>& test) {
    if (test) {
        tests_[test->getName()] = test;
    }
}

} // namespace oink_judge::test_node
