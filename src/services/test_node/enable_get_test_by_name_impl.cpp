#include "services/test_node/enable_get_test_by_name_impl.h"

namespace oink_judge::services::test_node {

enable_get_test_by_name_impl::enable_get_test_by_name_impl() = default;

std::shared_ptr<Test> enable_get_test_by_name_impl::get_test_by_name(const std::string &name) {
    auto it = _tests.find(name);
    if (it != _tests.end()) {
        return it->second;
    }
    return nullptr;
}

void enable_get_test_by_name_impl::add_test(const std::shared_ptr<Test> &test) {
    if (test) {
        _tests[test->get_name()] = test;
    }
}

} // namespace oink_judge::services::test_node
