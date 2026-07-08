#include <oink_judge/database/config_utils.h>
#include <oink_judge/database/connection_pool.h>
#include <oink_judge/database/statements.h>

#include <boost/asio/co_spawn.hpp>
#include <boost/asio/io_context.hpp>
#include <gtest/gtest.h>

#include <memory>
#include <string>
#include <utility>

using boost::asio::awaitable;
using oink_judge::database::ConnectionPool;
using oink_judge::database::DatabaseConfig;
using oink_judge::database::Statement;
using oink_judge::database::StatementsBlock;

namespace {

auto makeTestConfig() -> DatabaseConfig {
    return DatabaseConfig{
        .host = "127.0.0.1",
        .port = 5432, // NOLINT
        .username = "user",
        .password = "secret",
        .database_name = "db",
        .pool_min = 1,
        .pool_max = 2,
    };
}

auto makeUsersBlock() -> StatementsBlock {
    StatementsBlock block("users");
    block.addStatement(Statement("users__select", "SELECT 1"));
    return block;
}

auto runAwaitableVoid(awaitable<void> task) -> void {
    boost::asio::io_context io_context(1);

    boost::asio::co_spawn(
        io_context, [task = std::move(task)]() mutable -> awaitable<void> { co_await std::move(task); }, // NOLINT
        [&](const std::exception_ptr& exception) -> void {
            if (exception) {
                std::rethrow_exception(exception);
            }
        });

    io_context.run();
}

template <typename Result> auto runAwaitableAndRethrow(awaitable<Result> task) -> void {
    boost::asio::io_context io_context(1);
    std::exception_ptr error;

    boost::asio::co_spawn(
        io_context, [task = std::move(task)]() mutable -> awaitable<void> { (void)co_await std::move(task); }, // NOLINT
        [&](std::exception_ptr exception) -> void { error = std::move(exception); });

    io_context.run();
    if (error) {
        std::rethrow_exception(error);
    }
}

} // namespace

TEST(ConnectionPoolTest, PrepareAndUnprepareStatementsTrackBlocks) {
    auto pool = std::make_shared<ConnectionPool>(makeTestConfig());

    runAwaitableVoid(pool->prepareStatements(makeUsersBlock()));
    EXPECT_TRUE(pool->isPrepared("users"));

    runAwaitableVoid(pool->unprepareStatements("users"));
    EXPECT_FALSE(pool->isPrepared("users"));
}

TEST(ConnectionPoolTest, RejectsDuplicatePreparedBlock) {
    auto pool = std::make_shared<ConnectionPool>(makeTestConfig());
    runAwaitableVoid(pool->prepareStatements(makeUsersBlock()));
    EXPECT_THROW(runAwaitableVoid(pool->prepareStatements(makeUsersBlock())), std::runtime_error);
}

TEST(ConnectionPoolTest, AcquireFailsWithoutDatabaseConnection) {
    auto pool = std::make_shared<ConnectionPool>(makeTestConfig());
    EXPECT_THROW(runAwaitableAndRethrow(pool->acquire()), std::runtime_error);
}

TEST(ConnectionPoolTest, GetConnectionFailsWithoutDatabaseConnection) {
    auto pool = std::make_shared<ConnectionPool>(makeTestConfig());
    EXPECT_THROW(runAwaitableAndRethrow(pool->getConnection()), std::runtime_error);
}
