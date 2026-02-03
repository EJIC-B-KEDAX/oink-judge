#include "services/test_node/TestStorage.h"

#include "config/problem_config_utils.h"
#include "services/data_sender/ContentStorage.h"
#include "services/test_node/ProblemBuilder.hpp"

namespace oink_judge::services::test_node {

TestStorage& TestStorage::instance() {
    static TestStorage instance;
    return instance;
}

awaitable<std::shared_ptr<Test>> TestStorage::get_test(const std::string& problem_id) {
    co_await ensure_test_exists(problem_id);
    co_return _tests[problem_id];
}

awaitable<void> TestStorage::ensure_test_exists(const std::string& problem_id) {
    if (_tests.find(problem_id) != _tests.end()) {
        co_return;
    }

    co_await data_sender::ContentStorage::instance().ensure_content_exists("problem", problem_id);

    std::string builder_name = problem_config::getProblemBuilderName(problem_id);
    if (builder_name.empty()) {
        _tests[problem_id] = nullptr;
        co_return;
    }
    auto builder = ProblemBuilderFactory::instance().create(builder_name, problem_id);
    auto test = builder->build();

    _tests[problem_id] = test;
}

TestStorage::TestStorage() = default;

} // namespace oink_judge::services::test_node
