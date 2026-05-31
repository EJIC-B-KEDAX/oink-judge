#include "oink_judge/config/config.h"
#include "oink_judge/config/logger_utils.h"

#include <gtest/gtest.h>

using namespace oink_judge::config;

class LoggerConfigTest : public ::testing::Test {
  protected:
    auto SetUp() -> void override {
        resources_ = std::filesystem::path("resources") / "test_logger_config";
        LoggerConfigTest::loadConfig(resources_ / "good_config.json", resources_ / "good_credentials.json");
    }
    auto getResourcesPath() -> const std::filesystem::path& { return resources_; }

    static auto loadConfig(const std::filesystem::path& config, const std::filesystem::path& credentials) -> void {
        Config::setConfigFilePath(config);
        Config::setCredentialsFilePath(credentials);
        Config::reloadData();
    }

  private:
    std::filesystem::path resources_;
};

TEST_F(LoggerConfigTest, OutputStreamReturnsStdout) {
    auto out = getLoggerOutputStream();
    ASSERT_TRUE(out.has_value());
    EXPECT_EQ(*out, "stdout");
}

TEST_F(LoggerConfigTest, AllLogLevelsSize) {
    auto all = getAllLoggerLogLevels();
    ASSERT_TRUE(all.has_value());
    EXPECT_EQ(all->size(), 3);
}

TEST_F(LoggerConfigTest, LogLevelValues) {
    auto all = getAllLoggerLogLevels();
    ASSERT_TRUE(all.has_value());
    EXPECT_EQ(all->at("moduleA"), 2);
    EXPECT_EQ(all->at("moduleB"), 5);
    EXPECT_EQ(all->at("default"), 3);
}

TEST_F(LoggerConfigTest, GetLogLevelForExistingModule) {
    auto lvl = getLoggerLogLevel("moduleA");
    ASSERT_TRUE(lvl.has_value());
    EXPECT_EQ(*lvl, 2);
}

TEST_F(LoggerConfigTest, GetLogLevelForNonexistentModuleReturnsNullopt) {
    auto lvl = getLoggerLogLevel("nonexistent");
    EXPECT_FALSE(lvl.has_value());
}

TEST_F(LoggerConfigTest, ColorMapSize) {
    auto cmap = getLoggerColorMap();
    ASSERT_TRUE(cmap.has_value());
    EXPECT_EQ(cmap->size(), 2);
}

TEST_F(LoggerConfigTest, ColorMapValues) {
    auto cmap = getLoggerColorMap();
    ASSERT_TRUE(cmap.has_value());
    EXPECT_EQ(cmap->at("INFO"), "green");
    EXPECT_EQ(cmap->at("ERROR"), "red");
}

TEST_F(LoggerConfigTest, MinLocationLength) {
    auto min_loc = getLoggerMinLocationLength();
    ASSERT_TRUE(min_loc.has_value());
    EXPECT_EQ(*min_loc, 10);
}

TEST_F(LoggerConfigTest, MinModuleLength) {
    auto min_mod = getLoggerMinModuleLength();
    ASSERT_TRUE(min_mod.has_value());
    EXPECT_EQ(*min_mod, 8);
}

TEST_F(LoggerConfigTest, MissingLoggerOutputStreamReturnsNullopt) {
    LoggerConfigTest::loadConfig(getResourcesPath() / "no_logger.json", getResourcesPath() / "good_credentials.json");
    auto out = getLoggerOutputStream();
    EXPECT_FALSE(out.has_value());
}

TEST_F(LoggerConfigTest, MissingLoggerLogLevelsReturnsNullopt) {
    LoggerConfigTest::loadConfig(getResourcesPath() / "no_logger.json", getResourcesPath() / "good_credentials.json");
    auto all = getAllLoggerLogLevels();
    EXPECT_FALSE(all.has_value());
}

TEST_F(LoggerConfigTest, MissingLoggerConfigReturnsNullopt) {
    LoggerConfigTest::loadConfig(getResourcesPath() / "no_logger.json", getResourcesPath() / "good_credentials.json");
    auto cfg = getLoggerConfig();
    EXPECT_FALSE(cfg.has_value());
}
