#include "oink_judge/content_service/client/config_utils.h"
#include "oink_judge/content_service/client/content_service_stub.h"

#include <oink_judge/config/config.h>
#include <oink_judge/utils/grpc/base_types.h>

#include <gtest/gtest.h>

#include <filesystem>
#include <memory>
#include <string>

using namespace oink_judge::content_service;
using oink_judge::config::Config;
namespace fs = std::filesystem;

namespace {

class TestStub : public ContentServiceStub {
  public:
    auto getManifest(std::string /*content_type*/, std::string /*content_id*/)
        -> awaitable<tl::expected<json, grpc::Status>> override {
        co_return json::object();
    }

    auto getFile(std::string /*content_type*/, std::string /*content_id*/, std::string /*file_path*/)
        -> awaitable<tl::expected<std::string, grpc::Status>> override {
        co_return std::string{};
    }

    auto createFile(std::string /*content_type*/, std::string /*content_id*/, std::string /*file_path*/,
                    std::string /*file_content*/) -> awaitable<tl::expected<void, grpc::Status>> override {
        co_return tl::expected<void, grpc::Status>{};
    }

    auto updateFile(std::string /*content_type*/, std::string /*content_id*/, std::string /*file_path*/,
                    std::string /*file_content*/) -> awaitable<tl::expected<void, grpc::Status>> override {
        co_return tl::expected<void, grpc::Status>{};
    }

    auto deleteFile(std::string /*content_type*/, std::string /*content_id*/, std::string /*file_path*/)
        -> awaitable<tl::expected<void, grpc::Status>> override {
        co_return tl::expected<void, grpc::Status>{};
    }
};

auto registerTestStubType() -> void {
    ContentServiceStubFactory::instance().registerType(
        "test_stub",
        [](const std::string& /*params*/) -> std::unique_ptr<ContentServiceStub> { return std::make_unique<TestStub>(); });
}

} // namespace

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

// ---------------------------------------------------------------------------
// getContentStorageStubType
// ---------------------------------------------------------------------------

TEST_F(ClientConfigUtilsTest, MissingStubTypeReturnsNulloptForType) {
    loadConfig("config_no_stub.json");
    EXPECT_FALSE(getContentStorageStubType().has_value());
}

TEST_F(ClientConfigUtilsTest, NonStringStubTypeReturnsNulloptForType) {
    loadConfig("config_bad_stub_type.json");
    EXPECT_FALSE(getContentStorageStubType().has_value());
}

TEST_F(ClientConfigUtilsTest, RegisteredStubTypeIsReadFromConfig) {
    loadConfig("config_test_stub.json");

    auto stub_type_opt = getContentStorageStubType();

    ASSERT_TRUE(stub_type_opt.has_value());
    EXPECT_EQ(*stub_type_opt, "test_stub");
}

// ---------------------------------------------------------------------------
// getContentStorageStub
// ---------------------------------------------------------------------------

TEST_F(ClientConfigUtilsTest, MissingStubTypeReturnsNullopt) {
    loadConfig("config_no_stub.json");
    EXPECT_FALSE(getContentStorageStub().has_value());
}

TEST_F(ClientConfigUtilsTest, NonStringStubTypeReturnsNullopt) {
    loadConfig("config_bad_stub_type.json");
    EXPECT_FALSE(getContentStorageStub().has_value());
}

TEST_F(ClientConfigUtilsTest, RegisteredStubTypeIsCreatedFromConfig) {
    registerTestStubType();
    loadConfig("config_test_stub.json");

    auto stub_opt = getContentStorageStub();

    ASSERT_TRUE(stub_opt.has_value());
    ASSERT_NE(*stub_opt, nullptr);
    EXPECT_NE(dynamic_cast<TestStub*>(stub_opt->get()), nullptr);
}

// ---------------------------------------------------------------------------
// channel_stub factory registration
// ---------------------------------------------------------------------------

TEST_F(ClientConfigUtilsTest, ChannelStubRejectsMissingParameter) {
    registerContentServiceChannelStubType();
    EXPECT_THROW((void)ContentServiceStubFactory::instance().create("content_service_stub"), std::invalid_argument);
}

TEST_F(ClientConfigUtilsTest, ChannelStubRejectsTooManyParameters) {
    registerContentServiceChannelStubType();
    EXPECT_THROW((void)ContentServiceStubFactory::instance().create("content_service_stub(first,second)"), std::invalid_argument);
}

TEST_F(ClientConfigUtilsTest, ChannelStubIsCreatedWithRegisteredChannelType) {
    oink_judge::utils::grpc::registerChannelType();
    oink_judge::utils::grpc::registerInsecureCredentialsType();
    registerContentServiceChannelStubType();

    auto stub = ContentServiceStubFactory::instance().create("content_service_stub(channel(127.0.0.1:12345,insecure))");

    EXPECT_NE(stub, nullptr);
}
