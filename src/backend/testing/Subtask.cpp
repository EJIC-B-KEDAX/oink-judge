#include "backend/testing/Subtask.h"

namespace oink_judge::backend::testing {

Subtask::Subtask(const std::string &subtask_id) : _subtask_id(subtask_id) {}

std::string Subtask::get_subtask_id() const {
    return _subtask_id;
}

const std::vector<Test*> &Subtask::get_tests() const {
    return _tests;
}

void Subtask::push_test(Test *test) {
    _tests.push_back(test);
}

std::vector<Test*> &Subtask::access_tests() {
    return _tests;
}

} // namespace oink_judge::backend::testing
