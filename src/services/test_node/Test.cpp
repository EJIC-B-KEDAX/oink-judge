#include "services/test_node/Test.h"

namespace oink_judge::services::test_node {

Test::Test(const std::string &test_id) : _test_id(test_id) {}

std::string Test::get_test_id() const {
    return _test_id;
}

} // namespace oink_judge::services::test_node
