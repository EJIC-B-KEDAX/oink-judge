#include <oink_judge/config/config.h>
#include <oink_judge/database/config_utils.h>
#include <oink_judge/database/connection_pool.h>
#include <oink_judge/database/database_executor_interface.h>
#include <oink_judge/database/execute_options.h>
#include <oink_judge/database/query_param.h>
#include <oink_judge/database/statements.h>

#include <boost/asio/co_spawn.hpp>
#include <boost/asio/io_context.hpp>
#include <gtest/gtest.h>

#include <filesystem>
#include <memory>
#include <string>
#include <utility>

using boost::asio::awaitable;
using oink_judge::config::Config;
using oink_judge::database::ConnectionPool;
using oink_judge::database::createDatabaseExecutor;
using oink_judge::database::DatabaseExecutorFactory;
using oink_judge::database::DatabaseExecutorInterface;
using oink_judge::database::ExecuteOptions;
using oink_judge::database::getDefaultExecuteOptions;
using oink_judge::database::getDefaultExecutor;
using oink_judge::database::getDefaultExecutorName;
using oink_judge::database::makeQueryParams;
using oink_judge::database::QueryParam;
using oink_judge::database::QueryResult;
using oink_judge::database::registerDatabaseExecutorTypes;
using oink_judge::database::Statement;
using oink_judge::database::StatementsBlock;

namespace {

class DatabaseExecutorConfigTest : public ::testing::Test {
  protected:
    void SetUp() override {
        resources_ = std::filesystem::path("resources");
        Config::setConfigFilePath(resources_ / "good_config.json");
        Config::setCredentialsFilePath(resources_ / "good_credentials.json");
        Config::reloadData();
        registerDatabaseExecutorTypes();
    }

  private:
    std::filesystem::path resources_;
};

template <typename Result> auto runAwaitable(awaitable<Result> task) -> Result {
    boost::asio::io_context io_context(1);
    Result result{};

    boost::asio::co_spawn(
        io_context,
        [task = std::move(task), &result]() mutable -> awaitable<void> { result = co_await std::move(task); }, // NOLINT
        [&](const std::exception_ptr& exception) {
            if (exception) {
                std::rethrow_exception(exception);
            }
        });

    io_context.run();
    return result;
}

auto runAwaitableVoid(awaitable<void> task) -> void {
    boost::asio::io_context io_context(1);

    boost::asio::co_spawn(
        io_context, [task = std::move(task)]() mutable -> awaitable<void> { co_await std::move(task); }, // NOLINT
        [&](const std::exception_ptr& exception) {
            if (exception) {
                std::rethrow_exception(exception);
            }
        });

    io_context.run();
}

class RecordingExecutor : public DatabaseExecutorInterface {
  public:
    std::string last_stmt;
    std::vector<QueryParam> last_params;
    ExecuteOptions last_options{};
    int execute_calls = 0;
    int execute_sql_calls = 0;
    int get_connection_calls = 0;
    int prepare_calls = 0;
    int unprepare_calls = 0;
    std::string last_prepared_block;
    std::string last_unprepared_block;

    auto execute(ExecuteOptions options, std::string stmt, std::vector<QueryParam> params) -> awaitable<QueryResult> override {
        ++execute_calls;
        last_options = options;
        last_stmt = std::move(stmt);
        last_params = std::move(params);
        co_return QueryResult{};
    }

    auto executeSQL(ExecuteOptions options, std::string sql, std::vector<QueryParam> params) -> awaitable<QueryResult> override {
        ++execute_sql_calls;
        last_options = options;
        last_stmt = std::move(sql);
        last_params = std::move(params);
        co_return QueryResult{};
    }

    auto getConnection() -> awaitable<std::shared_ptr<DatabaseExecutorInterface>> override {
        ++get_connection_calls;
        co_return shared_from_this();
    }

    auto quote(std::string value) -> awaitable<std::string> override { co_return "'" + value + "'"; }

    auto prepareStatements(StatementsBlock statements_block) -> awaitable<void> override {
        ++prepare_calls;
        last_prepared_block = statements_block.name();
        co_return;
    }

    auto unprepareStatements(std::string block_name) -> awaitable<void> override {
        ++unprepare_calls;
        last_unprepared_block = std::move(block_name);
        co_return;
    }

    [[nodiscard]] auto isPrepared(const std::string& block_name) const -> bool override {
        return block_name == last_prepared_block;
    }
};

} // namespace

TEST_F(DatabaseExecutorConfigTest, ParsesDefaultExecuteOptionsFromConfig) {
    const auto options = getDefaultExecuteOptions();
    EXPECT_FALSE(options.read_only.value());
    EXPECT_EQ(options.timeout_sec, 5);
    EXPECT_EQ(options.retries, 2);
}

TEST_F(DatabaseExecutorConfigTest, ReadsDefaultExecutorNameFromConfig) { EXPECT_EQ(getDefaultExecutorName(), "connection_pool"); }

TEST_F(DatabaseExecutorConfigTest, FactoryCreatesConnectionPoolExecutor) {
    const auto executor = DatabaseExecutorFactory::instance().create(ConnectionPool::REGISTERED_NAME);
    ASSERT_NE(executor, nullptr);
    EXPECT_NE(dynamic_cast<ConnectionPool*>(executor.get()), nullptr);
}

TEST_F(DatabaseExecutorConfigTest, CreateAndDefaultExecutorReturnConnectionPool) {
    const auto created = createDatabaseExecutor(ConnectionPool::REGISTERED_NAME);
    const auto default_executor = getDefaultExecutor();
    ASSERT_NE(created, nullptr);
    ASSERT_NE(default_executor, nullptr);
    EXPECT_NE(dynamic_cast<ConnectionPool*>(created.get()), nullptr);
    EXPECT_NE(dynamic_cast<ConnectionPool*>(default_executor.get()), nullptr);
}

TEST(DatabaseExecutorTest, MakeQueryParamsBuildsVector) {
    const auto params = makeQueryParams(std::string{"alice"}, std::int64_t{42});
    ASSERT_EQ(params.size(), 2U);
    EXPECT_EQ(std::get<std::string>(params[0]), "alice");
    EXPECT_EQ(std::get<std::int64_t>(params[1]), 42);
}

TEST(DatabaseExecutorTest, MakeQueryParamsEmptyVector) { EXPECT_TRUE(makeQueryParams().empty()); }

TEST(DatabaseExecutorTest, InterfaceExecuteStoresVectorParameters) {
    auto executor = std::make_shared<RecordingExecutor>();
    const ExecuteOptions options{.read_only = true, .timeout_sec = 7, .retries = 1};
    runAwaitable(executor->execute(options, "users__select", makeQueryParams(std::string{"alice"}, std::int64_t{42}))); // NOLINT

    EXPECT_EQ(executor->execute_calls, 1);
    EXPECT_EQ(executor->last_stmt, "users__select");
    ASSERT_EQ(executor->last_params.size(), 2U);
    EXPECT_EQ(std::get<std::string>(executor->last_params[0]), "alice");
    EXPECT_EQ(std::get<std::int64_t>(executor->last_params[1]), 42);
    EXPECT_TRUE(executor->last_options.read_only.value());
    EXPECT_EQ(executor->last_options.timeout_sec, 7);
    EXPECT_EQ(executor->last_options.retries, 1);
}

TEST(DatabaseExecutorTest, InterfaceVariadicExecuteBuildsParameters) {
    auto executor = std::make_shared<RecordingExecutor>();
    DatabaseExecutorInterface& interface = *executor;
    runAwaitable(interface.execute("users__select", std::string{"alice"}, std::int64_t{42})); // NOLINT

    EXPECT_EQ(executor->execute_calls, 1);
    ASSERT_EQ(executor->last_params.size(), 2U);
    EXPECT_EQ(std::get<std::string>(executor->last_params[0]), "alice");
    EXPECT_EQ(std::get<std::int64_t>(executor->last_params[1]), 42);
}

TEST(DatabaseExecutorTest, InterfaceVariadicExecuteSQLBuildsParameters) {
    auto executor = std::make_shared<RecordingExecutor>();
    DatabaseExecutorInterface& interface = *executor;
    runAwaitable(interface.executeSQL("SELECT $1", std::int32_t{7})); // NOLINT

    EXPECT_EQ(executor->execute_sql_calls, 1);
    ASSERT_EQ(executor->last_params.size(), 1U);
    EXPECT_EQ(std::get<std::int32_t>(executor->last_params[0]), 7);
}

TEST(DatabaseExecutorTest, GetConnectionReturnsSharedExecutor) {
    auto executor = std::make_shared<RecordingExecutor>();
    const auto connection = runAwaitable(executor->getConnection());
    EXPECT_EQ(connection, executor);
    EXPECT_EQ(executor->get_connection_calls, 1);
}

TEST(DatabaseExecutorTest, PrepareStatementDelegatesToBlock) {
    auto executor = std::make_shared<RecordingExecutor>();
    runAwaitableVoid(executor->prepareStatement(Statement("users__select", "SELECT 1")));

    EXPECT_EQ(executor->prepare_calls, 1);
    EXPECT_EQ(executor->last_prepared_block, "users__select");
}

TEST(DatabaseExecutorTest, UnprepareStatementDelegatesToBlockName) {
    auto executor = std::make_shared<RecordingExecutor>();
    runAwaitableVoid(executor->unprepareStatement("users__select"));

    EXPECT_EQ(executor->unprepare_calls, 1);
    EXPECT_EQ(executor->last_unprepared_block, "users__select");
}

TEST(DatabaseExecutorTest, FactoryRejectsUnknownExecutorType) {
    registerDatabaseExecutorTypes();
    EXPECT_THROW((void)DatabaseExecutorFactory::instance().create("unknown"), std::runtime_error);
}
