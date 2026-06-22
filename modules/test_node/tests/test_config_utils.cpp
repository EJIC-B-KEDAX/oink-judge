#include "oink_judge/test_node/config_utils.h"

#include <oink_judge/config/config.h>

#include <gtest/gtest.h>

#include <filesystem>
#include <string>

using oink_judge::config::Config;
namespace fs = std::filesystem;

using namespace oink_judge::test_node;

class TestNodeConfigUtilsTest : public ::testing::Test {
  protected:
    auto SetUp() -> void override { resources_ = fs::path("resources") / "test_config_utils"; }

    auto loadConfig(const std::string& config_name) -> void {
        Config::setConfigFilePath(resources_ / config_name);
        Config::setCredentialsFilePath(resources_ / "good_credentials.json");
        Config::reloadData();
    }

  private:
    fs::path resources_;
};

TEST_F(TestNodeConfigUtilsTest, MyTestNodeIdIsReadFromConfig) {
    loadConfig("good_config.json");

    auto id = getMyTestNodeId();

    ASSERT_TRUE(id.has_value());
    EXPECT_EQ(*id, "test_node_1");
}

TEST_F(TestNodeConfigUtilsTest, MyTestNodeTypeIsReadFromConfig) {
    loadConfig("good_config.json");

    auto type = getMyTestNodeType();

    ASSERT_TRUE(type.has_value());
    EXPECT_EQ(*type, "default");
}

TEST_F(TestNodeConfigUtilsTest, TestingLogFilePathIsReadFromConfig) {
    loadConfig("good_config.json");

    auto path = getTestingLogFilePath("meta_file");

    ASSERT_TRUE(path.has_value());
    EXPECT_EQ(*path, fs::path("logs/meta.txt"));
}

TEST_F(TestNodeConfigUtilsTest, UnknownTestingLogKeyReturnsNullopt) {
    loadConfig("good_config.json");

    EXPECT_FALSE(getTestingLogFilePath("unknown_key").has_value());
}

TEST_F(TestNodeConfigUtilsTest, MissingStubTypeReturnsNullopt) {
    loadConfig("config_no_stub.json");

    EXPECT_FALSE(getQueueManagerServiceStubType().has_value());
}

TEST_F(TestNodeConfigUtilsTest, NonStringStubTypeReturnsNullopt) {
    loadConfig("config_bad_stub_type.json");

    EXPECT_FALSE(getQueueManagerServiceStubType().has_value());
}

TEST_F(TestNodeConfigUtilsTest, StubTypeIsReadFromConfig) {
    loadConfig("good_config.json");

    auto stub_type = getQueueManagerServiceStubType();

    ASSERT_TRUE(stub_type.has_value());
    EXPECT_EQ(*stub_type, "queue_manager_service_stub");
}
