#include <oink_judge/database/connection_pool.h>

#include <boost/asio/co_spawn.hpp>
#include <boost/asio/io_context.hpp>
#include <gtest/gtest.h>

#include <string>
#include <utility>

using boost::asio::awaitable;
using oink_judge::database::ConnectionPool;

namespace {

template <typename Result>
auto expectRuntimeErrorContaining(awaitable<Result> task, const std::string& expected_substring) -> void {
    boost::asio::io_context io_context(1);
    std::exception_ptr error;

    boost::asio::co_spawn(
        io_context, [task = std::move(task)]() mutable -> awaitable<void> { (void)co_await std::move(task); }, // NOLINT
        [&](std::exception_ptr exception) { error = std::move(exception); });

    io_context.run();
    ASSERT_TRUE(error);

    try {
        std::rethrow_exception(error);
        FAIL() << "expected std::runtime_error";
    } catch (const std::runtime_error& exception) {
        EXPECT_NE(std::string(exception.what()).find(expected_substring), std::string::npos);
    }
}

} // namespace

TEST(ConnectionPoolTest, AcquireAutoInitializesWithoutConfig) {
    expectRuntimeErrorContaining(ConnectionPool::instance().acquire(), "Could not open config file:");
}
