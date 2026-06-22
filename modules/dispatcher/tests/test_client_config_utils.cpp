#include "oink_judge/dispatcher/client/config_utils.h"

#include <oink_judge/config/config.h>

#include <gtest/gtest.h>

#include <filesystem>
#include <string>

using oink_judge::config::Config;
namespace fs = std::filesystem;

using namespace oink_judge::dispatcher;

class ClientConfigUtilsTest : public ::testing::Test {
  protected:
    auto SetUp() -> void override { resources_ = fs::path("resources") / "test_client_config_utils"; }

    auto loadConfig(const std::string& config_name) -> void {
        Config::setConfigFilePath(resources_ / config_name);
        Config::setCredentialsFilePath(resources_ / "good_credentials.json");
        Config::reloadData();
    }

  private:
    fs::path resources_;
};

TEST_F(ClientConfigUtilsTest, MissingStubTypeReturnsNullopt) {
    loadConfig("config_no_stub.json");
    EXPECT_FALSE(getDispatcherStubType().has_value());
}

TEST_F(ClientConfigUtilsTest, NonStringStubTypeReturnsNullopt) {
    loadConfig("config_bad_stub_type.json");
    EXPECT_FALSE(getDispatcherStubType().has_value());
}

TEST_F(ClientConfigUtilsTest, StubTypeIsReadFromConfig) {
    loadConfig("config_test_stub.json");

    auto stub_type_opt = getDispatcherStubType();

    ASSERT_TRUE(stub_type_opt.has_value());
    EXPECT_EQ(*stub_type_opt, "test_stub");
}
