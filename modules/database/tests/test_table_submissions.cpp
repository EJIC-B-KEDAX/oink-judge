#include <oink_judge/config/config.h>
#include <oink_judge/database/table_submissions.h>

#include <boost/asio/co_spawn.hpp>
#include <boost/asio/io_context.hpp>
#include <gtest/gtest.h>

#include <filesystem>
#include <utility>

using boost::asio::awaitable;
using oink_judge::config::Config;
using oink_judge::database::TableSubmissions;

namespace {

template <typename Result>
auto runAwaitable(awaitable<Result> task) -> Result {
    boost::asio::io_context io_context(1);
    Result value{};

    boost::asio::co_spawn(
        io_context, [&value, task = std::move(task)]() mutable -> awaitable<void> { value = co_await std::move(task); }, // NOLINT
        [&](const std::exception_ptr& exception) -> void {
            if (exception) {
                std::rethrow_exception(exception);
            }
        });

    io_context.run();
    return value;
}

} // namespace

class TableSubmissionsTest : public ::testing::Test {
  protected:
    void SetUp() override {
        resources_ = std::filesystem::path("resources");
        Config::setConfigFilePath(resources_ / "good_config.json");
        Config::setCredentialsFilePath(resources_ / "good_credentials.json");
        Config::reloadData();
    }

  private:
    std::filesystem::path resources_;
};

TEST_F(TableSubmissionsTest, LoadReturnsNulloptWhenDatabaseIsUnavailable) {
    const auto submissions = runAwaitable(TableSubmissions::instance().loadSubmissionsByUserAndProblem("user", "problem"));
    EXPECT_FALSE(submissions.has_value());
}
