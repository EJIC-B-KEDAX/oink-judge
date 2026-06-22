#include <oink_judge/config/config.h>
#include <oink_judge/database/config_utils.h>

#include <gtest/gtest.h>

#include <filesystem>

using oink_judge::config::Config;
using oink_judge::database::buildConnectionString;
using oink_judge::database::DatabaseConfig;
using oink_judge::database::getDatabaseConfig;

class DatabaseConfigUtilsTest : public ::testing::Test {
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

TEST_F(DatabaseConfigUtilsTest, ParsesDatabaseConfigWithPoolSettings) {
    const auto config = getDatabaseConfig();
    ASSERT_TRUE(config.has_value());
    EXPECT_EQ(config->host, "127.0.0.1");
    EXPECT_EQ(config->port, 5432);
    EXPECT_EQ(config->username, "my_username");
    EXPECT_EQ(config->database_name, "my_dbname");
    EXPECT_EQ(config->pool_min, 1);
    EXPECT_EQ(config->pool_max, 3);
}

TEST_F(DatabaseConfigUtilsTest, BuildsConnectionString) {
    DatabaseConfig config{
        .host = "localhost",
        .port = 5432, // NOLINT
        .username = "user",
        .password = "secret",
        .database_name = "db",
        .pool_min = 1,
        .pool_max = 2,
        .connect_timeout_sec = 15, // NOLINT
    };

    const auto conninfo = buildConnectionString(config);
    EXPECT_NE(conninfo.find("host=localhost"), std::string::npos);
    EXPECT_NE(conninfo.find("connect_timeout=15"), std::string::npos);
}
