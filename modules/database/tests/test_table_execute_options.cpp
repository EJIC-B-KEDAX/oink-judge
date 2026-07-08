#include <oink_judge/config/config.h>
#include <oink_judge/database/connection_pool.h>
#include <oink_judge/database/table_execute_options.h>

#include <gtest/gtest.h>

#include <filesystem>
#include <memory>

using oink_judge::config::Config;
using oink_judge::database::ConnectionPool;
using oink_judge::database::ExecuteOptions;
using oink_judge::database::getDefaultExecutorOption;
using oink_judge::database::TableExecuteOptions;

class TableExecuteOptionsTest : public ::testing::Test {
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

TEST_F(TableExecuteOptionsTest, GetDefaultExecutorOptionReturnsConnectionPool) {
    const auto executor = getDefaultExecutorOption();
    ASSERT_NE(executor, nullptr);
    EXPECT_NE(dynamic_cast<ConnectionPool*>(executor.get()), nullptr);
}

TEST_F(TableExecuteOptionsTest, FillWithDefaultsUsesConfigAndDefaultExecutor) {
    TableExecuteOptions options{};
    options.fillWithDefaults();

    ASSERT_NE(options.executor, nullptr);
    EXPECT_FALSE(options.execute_options.read_only.value());
    EXPECT_EQ(options.execute_options.timeout_sec, 5);
    EXPECT_EQ(options.execute_options.retries, 2);
}

TEST_F(TableExecuteOptionsTest, FillWithDefaultsPreservesExplicitExecutor) {
    auto custom_executor = std::make_shared<ConnectionPool>();
    TableExecuteOptions options{.execute_options = {.read_only = true}, .executor = custom_executor};
    options.fillWithDefaults();

    EXPECT_EQ(options.executor, custom_executor);
    EXPECT_TRUE(options.execute_options.read_only.value());
    EXPECT_EQ(options.execute_options.timeout_sec, 5);
    EXPECT_EQ(options.execute_options.retries, 2);
}

TEST_F(TableExecuteOptionsTest, FillWithCustomDefaults) {
    auto custom_executor = std::make_shared<ConnectionPool>();
    TableExecuteOptions options{};
    options.fillWithDefaults(
        {.execute_options = {.read_only = true, .timeout_sec = 1, .retries = 0}, .executor = custom_executor});

    EXPECT_EQ(options.executor, custom_executor);
    EXPECT_TRUE(options.execute_options.read_only.value());
    EXPECT_EQ(options.execute_options.timeout_sec, 1);
    EXPECT_EQ(options.execute_options.retries, 0);
}
