#pragma once
#include "Test.hpp"

namespace oink_judge::services::test_node {

class TestStorage {
public:
    static TestStorage &instance();

    TestStorage(const TestStorage &) = delete;
    TestStorage &operator=(const TestStorage &) = delete;

    void get_test(const std::string &problem_id, std::function<void(std::error_code, std::shared_ptr<Test>)> callback);

private:
    void ensure_test_exists(const std::string &problem_id, std::function<void(std::error_code)> callback);

    TestStorage();

    std::map<std::string, std::shared_ptr<Test>> _tests;
};

} // namespace oink_judge::services::test_node
