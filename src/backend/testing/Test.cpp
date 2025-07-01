#include "backend/testing/Test.h"

namespace oink_judge::backend::testing {

Test::Test(const std::string &test_id) : _test_id(test_id) {}

std::string Test::get_test_id() const {
    return _test_id;
}

} // namespace oink_judge::backend::testing
