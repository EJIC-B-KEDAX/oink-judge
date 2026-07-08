#include <oink_judge/config/config.h>
#include <oink_judge/database/config_utils.h>
#include <oink_judge/database/connection_pool.h>
#include <oink_judge/database/execute_options.h>
#include <oink_judge/database/libpq_connection.h>
#include <oink_judge/database/statements.h>
#include <oink_judge/database/table_submissions.h>

#include <boost/asio/co_spawn.hpp>
#include <boost/asio/detached.hpp>
#include <boost/asio/io_context.hpp>
#include <boost/asio/post.hpp>
#include <boost/asio/steady_timer.hpp>
#include <boost/asio/this_coro.hpp>
#include <boost/asio/use_awaitable.hpp>
#include <gtest/gtest.h>

#include <chrono>
#include <cstdlib>
#include <filesystem>
#include <optional>

using boost::asio::awaitable;
using boost::asio::use_awaitable;
using oink_judge::config::Config;
using oink_judge::database::buildConnectionString;
using oink_judge::database::ConnectionPool;
using oink_judge::database::ExecuteOptions;
using oink_judge::database::LibpqConnection;
using oink_judge::database::QueryParam;
using oink_judge::database::QueryResult;
using oink_judge::database::Statement;
using oink_judge::database::TableSubmissions;

namespace {

class DatabaseIntegrationTest : public ::testing::Test {
  protected:
    void SetUp() override {
        if (std::getenv("OINK_JUDGE_RUN_DATABASE_TESTS") == nullptr) {
            GTEST_SKIP() << "Set OINK_JUDGE_RUN_DATABASE_TESTS=1 to run database integration tests";
        }

        const auto* config_dir = std::getenv("OINK_JUDGE_TEST_CONFIG_DIR");
        const std::filesystem::path resources =
            config_dir != nullptr ? std::filesystem::path(config_dir) : std::filesystem::path("resources");
        Config::setConfigFilePath(resources / "good_config.json");
        Config::setCredentialsFilePath(resources / "good_credentials.json");
        Config::reloadData();
    }
};

auto runIntegrationCase() -> awaitable<void> {
    auto pool = std::make_shared<ConnectionPool>();
    co_await pool->initialize();

    co_await pool->prepareStatement(Statement("integration__ping", "SELECT 1 AS value"));
    const auto first = co_await pool->execute(ExecuteOptions{}, "integration__ping", std::vector<QueryParam>{});
    const auto second = co_await pool->execute(ExecuteOptions{}, "integration__ping", std::vector<QueryParam>{});

    if (first.empty() || second.empty()) {
        throw std::runtime_error("expected query results");
    }

    if (!co_await TableSubmissions::instance().initialize()) {
        throw std::runtime_error("failed to initialize submissions table");
    }

    co_return;
}

auto executeSQLInto(std::shared_ptr<LibpqConnection> connection, std::string sql, std::optional<QueryResult>* result)
    -> awaitable<void> {
    *result = co_await connection->executeSQL(ExecuteOptions{}, std::move(sql), std::vector<QueryParam>{});
    co_return;
}

auto runSingleConnectionQueueCase() -> awaitable<void> {
    const auto config = oink_judge::database::getDatabaseConfig();
    if (!config.has_value()) {
        throw std::runtime_error("database config is not available");
    }

    auto connection = std::make_shared<LibpqConnection>();
    co_await connection->connectAsync(buildConnectionString(*config));

    const auto executor = co_await boost::asio::this_coro::executor;
    std::optional<QueryResult> first_result;
    std::optional<QueryResult> second_result;
    std::exception_ptr first_error;
    std::exception_ptr second_error;
    int completed = 0;

    boost::asio::co_spawn(executor, executeSQLInto(connection, "SELECT 1 AS value, pg_sleep(0.05)", &first_result),
                          [&](std::exception_ptr exception) -> void {
                              first_error = std::move(exception);
                              ++completed;
                          });

    co_await boost::asio::post(use_awaitable);

    boost::asio::co_spawn(executor, executeSQLInto(connection, "SELECT 2 AS value", &second_result),
                          [&](std::exception_ptr exception) -> void {
                              second_error = std::move(exception);
                              ++completed;
                          });

    while (completed < 2) {
        boost::asio::steady_timer timer(executor);
        timer.expires_after(std::chrono::milliseconds(1));
        co_await timer.async_wait(use_awaitable);
    }

    if (first_error) {
        std::rethrow_exception(first_error);
    }
    if (second_error) {
        std::rethrow_exception(second_error);
    }

    if (!first_result.has_value() || !second_result.has_value()) {
        throw std::runtime_error("expected both queued query results");
    }
    if (first_result->size() != 1U || second_result->size() != 1U) {
        throw std::runtime_error("expected one row from each queued query");
    }
    if ((*first_result)[0]["value"].as<int>() != 1 || (*second_result)[0]["value"].as<int>() != 2) {
        throw std::runtime_error("queued queries returned unexpected values");
    }

    co_return;
}

} // namespace

TEST_F(DatabaseIntegrationTest, ConnectionPoolExecutesPreparedStatement) {
    boost::asio::io_context io_context(1);
    boost::asio::co_spawn(io_context, []() -> awaitable<void> { co_await runIntegrationCase(); }, boost::asio::detached);
    io_context.run();
}

TEST_F(DatabaseIntegrationTest, LibpqConnectionQueuesConcurrentQueries) {
    boost::asio::io_context io_context(1);
    boost::asio::co_spawn(
        io_context, []() -> awaitable<void> { co_await runSingleConnectionQueueCase(); }, boost::asio::detached);
    io_context.run();
}
