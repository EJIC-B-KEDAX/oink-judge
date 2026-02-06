#pragma once
#include "oink_judge/test_node/test.hpp"

#include <boost/asio/awaitable.hpp>

namespace oink_judge::test_node {

using boost::asio::awaitable;

class TestStorage {
  public:
    static auto instance() -> TestStorage&;

    TestStorage(const TestStorage&) = delete;
    auto operator=(const TestStorage&) -> TestStorage& = delete;
    TestStorage(TestStorage&&) = delete;
    auto operator=(TestStorage&&) -> TestStorage& = delete;
    ~TestStorage() = default;

    [[nodiscard]] auto getTest(std::string problem_id) -> awaitable<std::shared_ptr<Test>>;

  private:
    auto ensureTestExists(std::string problem_id) -> awaitable<void>;

    TestStorage();

    std::map<std::string, std::shared_ptr<Test>> tests_;
};

} // namespace oink_judge::test_node
