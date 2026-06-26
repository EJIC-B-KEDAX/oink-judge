#include <oink_judge/database/table_submissions.h>

#include <boost/asio/co_spawn.hpp>
#include <boost/asio/io_context.hpp>
#include <gtest/gtest.h>

#include <optional>
#include <utility>

using boost::asio::awaitable;
using oink_judge::database::SubmissionRow;
using oink_judge::database::TableSubmissions;

namespace {

template <typename Result> auto runAwaitable(awaitable<Result> task) -> Result {
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

TEST(TableSubmissionsTest, LoadAutoInitializesWithoutConfig) {
    const auto submissions = runAwaitable(TableSubmissions::instance().loadSubmissionsByUserAndProblem("user", "problem"));
    EXPECT_FALSE(submissions.has_value());
}
