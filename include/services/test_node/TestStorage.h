#pragma once
#include "Test.hpp"
#include <boost/asio/awaitable.hpp>

namespace oink_judge::services::test_node {

using boost::asio::awaitable;

class TestStorage {
public:
    static TestStorage &instance();

    TestStorage(const TestStorage &) = delete;
    TestStorage &operator=(const TestStorage &) = delete;

    awaitable<std::shared_ptr<Test>> get_test(const std::string &problem_id);

private:
    awaitable<void> ensure_test_exists(const std::string &problem_id);

    TestStorage();

    std::map<std::string, std::shared_ptr<Test>> _tests;
};

} // namespace oink_judge::services::test_node
