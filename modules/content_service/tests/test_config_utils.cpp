#include "oink_judge/content_service/config_utils.h"

#include <oink_judge/config/config.h>

#include <gtest/gtest.h>

#include <filesystem>

using namespace oink_judge::content_service;
using oink_judge::config::Config;

class ConfigUtilsTest : public ::testing::Test {
  protected:
    auto SetUp() -> void override {
        resources_ = std::filesystem::path("resources") / "test_config_utils";
        Config::setConfigFilePath(resources_ / "good_config.json");
        Config::setCredentialsFilePath(resources_ / "good_credentials.json");
        Config::reloadData();
    }

    auto getResourcesPath() -> const std::filesystem::path& { return resources_; }

  private:
    std::filesystem::path resources_;
};

TEST_F(ConfigUtilsTest, KnownContentTypeReturnsPath) {
    auto dir = getContentDirectory("problem");
    ASSERT_TRUE(dir.has_value());
    EXPECT_FALSE(dir->empty());
}

TEST_F(ConfigUtilsTest, KnownContentTypeReturnsConfiguredPath) {
    auto dir = getContentDirectory("problem");
    ASSERT_TRUE(dir.has_value());
    EXPECT_EQ(std::filesystem::weakly_canonical(std::filesystem::absolute(*dir)),
              std::filesystem::weakly_canonical(std::filesystem::absolute(getResourcesPath() / "problems")));
}

TEST_F(ConfigUtilsTest, UnknownContentTypeReturnsNullopt) { EXPECT_FALSE(getContentDirectory("nonexistent").has_value()); }

TEST_F(ConfigUtilsTest, ContentTypePluralizedForDirectoryLookup) {
    // "problem" → looks up "problems" in config; "submission" → "submissions"
    // Only "problems" is in the config, so "submission" must return nullopt
    EXPECT_FALSE(getContentDirectory("submission").has_value());
}
