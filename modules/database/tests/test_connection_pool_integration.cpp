#include <oink_judge/config/config.h>
#include <oink_judge/database/table_submissions.h>
#include <oink_judge/database/connection_pool.h>
#include <oink_judge/database/query.h>

#include <boost/asio/co_spawn.hpp>
#include <boost/asio/detached.hpp>
#include <boost/asio/io_context.hpp>
#include <boost/asio/use_awaitable.hpp>

#include <cstdlib>
#include <filesystem>

#include <gtest/gtest.h>

using boost::asio::awaitable;
using boost::asio::use_awaitable;
using oink_judge::config::Config;
using oink_judge::database::TableSubmissions;
using oink_judge::database::ConnectionPool;
using oink_judge::database::executeReadOnly;

namespace {

class DatabaseIntegrationTest : public ::testing::Test {
  protected:
    void SetUp() override {
        if (std::getenv("OINK_JUDGE_RUN_DATABASE_TESTS") == nullptr) {
            GTEST_SKIP() << "Set OINK_JUDGE_RUN_DATABASE_TESTS=1 to run database integration tests";
        }

        const auto* config_dir = std::getenv("OINK_JUDGE_TEST_CONFIG_DIR");
        const std::filesystem::path resources =
            config_dir != nullptr ? std::filesystem::path(config_dir) : std::filesystem::path("test_async_database");
        Config::setConfigFilePath(resources / "good_config.json");
        Config::setCredentialsFilePath(resources / "good_credentials.json");
        Config::reloadData();
    }
};

auto runIntegrationCase() -> awaitable<void> {
    auto& pool = ConnectionPool::instance();
    co_await pool.initialize();

    pool.prepareStatement("integration__ping", "SELECT 1 AS value");
    const auto first = co_await executeReadOnly(pool, "integration__ping", std::vector<oink_judge::database::QueryParam>{});
    const auto second = co_await executeReadOnly(pool, "integration__ping", std::vector<oink_judge::database::QueryParam>{});

    if (first.empty() || second.empty()) {
        throw std::runtime_error("expected query results");
    }

    if (!co_await TableSubmissions::instance().initialize()) {
        throw std::runtime_error("failed to initialize submissions table");
    }
}

} // namespace

TEST_F(DatabaseIntegrationTest, ConnectionPoolExecutesPreparedStatement) {
    boost::asio::io_context io_context(1);
    boost::asio::co_spawn(
        io_context,
        []() -> awaitable<void> {
            co_await runIntegrationCase();
        },
        boost::asio::detached);
    io_context.run();
}
