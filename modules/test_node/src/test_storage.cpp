#include "oink_judge/test_node/test_storage.h"

#include "oink_judge/config/problem_config_utils.h"
#include "oink_judge/test_node/problem_builder.hpp"

#include <oink_judge/content_service/content_storage.h>

namespace oink_judge::test_node {

auto TestStorage::instance() -> TestStorage& {
    static TestStorage instance;
    return instance;
}

auto TestStorage::getTest(std::string problem_id) -> awaitable<std::shared_ptr<Test>> {
    co_await ensureTestExists(problem_id);
    co_return tests_[problem_id];
}

auto TestStorage::ensureTestExists(std::string problem_id) -> awaitable<void> {
    if (tests_.contains(problem_id)) {
        co_return;
    }

    co_await content_service::ContentStorage::instance().ensureContentExists("problem", problem_id);

    std::string builder_name = problem_config::getProblemBuilderName(problem_id).value_or("");
    if (builder_name.empty()) {
        tests_[problem_id] = nullptr;
        co_return;
    }
    auto builder = ProblemBuilderFactory::instance().create(builder_name, problem_id);
    auto test = builder->build();

    tests_[problem_id] = test;
}

TestStorage::TestStorage() = default;

} // namespace oink_judge::test_node
