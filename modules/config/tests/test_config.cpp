#include "oink_judge/config/common_utils.h"
#include "oink_judge/config/config.h"

#include <gtest/gtest.h>

using namespace oink_judge::config;

class ConfigTest : public ::testing::Test {
  protected:
    auto SetUp() -> void override {
        resources_ = std::filesystem::path("resources") / "test_config";
        Config::setConfigFilePath(resources_ / "good_config.json");
        Config::setCredentialsFilePath(resources_ / "good_credentials.json");
        Config::reloadData();
    }

    auto getResourcesPath() -> const std::filesystem::path& { return resources_; }

  private:
    std::filesystem::path resources_;
};

TEST_F(ConfigTest, DirectoryPathReturnsCorrectPath) {
    auto dir = getDirectoryPath("problems");
    ASSERT_TRUE(dir.has_value());
    EXPECT_EQ(std::filesystem::weakly_canonical(std::filesystem::absolute(dir.value())),
              std::filesystem::weakly_canonical(getResourcesPath() / "problems"));
}

TEST_F(ConfigTest, DirectoryPathReturnsNulloptForMissingKey) { EXPECT_FALSE(getDirectoryPath("non_existent_key").has_value()); }

TEST_F(ConfigTest, MalformedJsonThrows) {
    Config::setConfigFilePath(getResourcesPath() / "bad_malformed.json");
    EXPECT_ANY_THROW(Config::reloadData());
}

TEST_F(ConfigTest, MissingConfigThrows) {
    Config::setConfigFilePath(getResourcesPath() / "non_existent.json");
    EXPECT_ANY_THROW(Config::reloadData());
}
