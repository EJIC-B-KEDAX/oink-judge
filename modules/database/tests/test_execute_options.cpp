#include <oink_judge/config/config.h>
#include <oink_judge/database/execute_options.h>

#include <gtest/gtest.h>

#include <filesystem>

using oink_judge::config::Config;
using oink_judge::database::ExecuteOptions;
using oink_judge::database::getDefaultExecuteOptions;

class ExecuteOptionsTest : public ::testing::Test {
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

TEST_F(ExecuteOptionsTest, LoadsDefaultsFromConfig) {
    const auto options = getDefaultExecuteOptions();
    EXPECT_FALSE(options.read_only.value());
    EXPECT_EQ(options.timeout_sec, 5);
    EXPECT_EQ(options.retries, 2);
}

TEST_F(ExecuteOptionsTest, FillWithDefaultsUsesConfigValues) {
    ExecuteOptions options{};
    options.fillWithDefaults();
    EXPECT_FALSE(options.read_only.value());
    EXPECT_EQ(options.timeout_sec, 5);
    EXPECT_EQ(options.retries, 2);
}

TEST_F(ExecuteOptionsTest, FillWithDefaultsPreservesExplicitValues) {
    ExecuteOptions options{.read_only = true, .timeout_sec = 9, .retries = 1}; // NOLINT
    options.fillWithDefaults();
    EXPECT_TRUE(options.read_only.value());
    EXPECT_EQ(options.timeout_sec, 9);
    EXPECT_EQ(options.retries, 1);
}

TEST_F(ExecuteOptionsTest, FillWithDefaultsFillsOnlyMissingFields) {
    ExecuteOptions options{.timeout_sec = 9}; // NOLINT
    options.fillWithDefaults();
    EXPECT_FALSE(options.read_only.value());
    EXPECT_EQ(options.timeout_sec, 9);
    EXPECT_EQ(options.retries, 2);
}

TEST_F(ExecuteOptionsTest, FillWithCustomDefaults) {
    ExecuteOptions options{.read_only = std::nullopt, .timeout_sec = std::nullopt, .retries = std::nullopt};
    options.fillWithDefaults({.read_only = true, .timeout_sec = 3, .retries = 4});
    EXPECT_TRUE(options.read_only.value());
    EXPECT_EQ(options.timeout_sec, 3);
    EXPECT_EQ(options.retries, 4);
}
