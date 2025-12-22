#include "services/test_node/TestStorage.h"
#include "services/test_node/ProblemBuilder.hpp"
#include "services/data_sender/ContentStorage.h"
#include "config/problem_config_utils.h"

namespace oink_judge::services::test_node {

TestStorage &TestStorage::instance() {
    static TestStorage instance;
    return instance;
}

void TestStorage::get_test(const std::string &problem_id, std::function<void(std::error_code, std::shared_ptr<Test>)> callback) {
    ensure_test_exists(problem_id, [this, problem_id, callback](std::error_code ec) {
        if (ec) {
            callback(ec, nullptr);
            return;
        }
        callback(std::error_code{}, _tests[problem_id]);
    });
}

void TestStorage::ensure_test_exists(const std::string &problem_id, std::function<void(std::error_code)> callback) {
    if (_tests.find(problem_id) != _tests.end()) {
        callback(std::error_code{});
        return;
    }

    data_sender::ContentStorage::instance().ensure_content_exists("problem", problem_id, [this, problem_id, callback](std::error_code ec) {
        if (ec) {
            callback(ec);
            return;
        }
        std::string builder_name = problem_config::get_problem_builder_name(problem_id);
        if (builder_name.empty()) {
            _tests[problem_id] = nullptr;
            callback(std::error_code{});
            return;
        }
        auto builder = ProblemBuilderFactory::instance().create(builder_name, problem_id);
        auto test = builder->build();

        _tests[problem_id] = test;
        callback(std::error_code{});
    });
}

TestStorage::TestStorage() = default;

} // namespace oink_judge::services::test_node
