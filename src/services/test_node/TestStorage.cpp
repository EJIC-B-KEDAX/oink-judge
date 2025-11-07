#include "services/test_node/TestStorage.h"
#include "services/test_node/ProblemBuilder.hpp"
#include "services/data_sender/ContentStorage.h"
#include "services/test_node/problem_utils.h"

namespace oink_judge::services::test_node {

TestStorage &TestStorage::instance() {
    static TestStorage instance;
    return instance;
}

std::shared_ptr<Test> TestStorage::get_test(const std::string &problem_id) {
    ensure_test_exists(problem_id);
    return _tests[problem_id];
}

void TestStorage::ensure_test_exists(const std::string &problem_id) {
    if (_tests.find(problem_id) != _tests.end()) return;

    data_sender::ContentStorage::instance().ensure_content_exists("problem", problem_id);

    std::string builder_name = get_problem_builder_name(problem_id);
    if (builder_name.empty()) {
        _tests[problem_id] = nullptr;
        return;
    }
    auto builder = ProblemBuilderFactory::instance().create(builder_name, problem_id);
    auto test = builder->build();

    _tests[problem_id] = test;
}

TestStorage::TestStorage() = default;

} // namespace oink_judge::services::test_node
