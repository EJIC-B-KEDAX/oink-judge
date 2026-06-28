#include "oink_judge/content_service/client/config_utils.h"
#include "oink_judge/content_service/client/content_service_stub.h"
#include "oink_judge/content_service/client/content_storage.h"

#include <oink_judge/config/config.h>
#include <oink_judge/utils/filesystem.h>
#include <oink_judge/utils/grpc/base_types.h>

#include <agrpc/grpc_context.hpp>
#include <agrpc/register_awaitable_rpc_handler.hpp>
#include <arpa/inet.h>
#include <boost/asio.hpp>
#include <grpcpp/grpcpp.h>
#include <gtest/gtest.h>
#include <netinet/in.h>
#include <nlohmann/json.hpp>
#include <sys/socket.h>
#include <sys/wait.h>
#include <tl/expected.hpp>
#include <unistd.h>

#include <csignal>
#include <cstdlib>
#include <filesystem>
#include <fstream>
#include <memory>
#include <string>

using namespace oink_judge::content_service;
using nlohmann::json;
using oink_judge::config::Config;
namespace fs = std::filesystem;

namespace {

constexpr auto SERVER_EXE_ENV = "CONTENT_SERVICE_TEST_SERVER";

auto freePort() -> int {
    int sock = socket(AF_INET, SOCK_STREAM, 0);
    if (sock < 0) {
        throw std::runtime_error("Failed to create socket for free port lookup");
    }
    sockaddr_in addr{};
    addr.sin_family = AF_INET;
    addr.sin_addr.s_addr = htonl(INADDR_LOOPBACK);
    addr.sin_port = 0;
    if (bind(sock, reinterpret_cast<sockaddr*>(&addr), sizeof(addr)) != 0) { // NOLINT
        close(sock);
        throw std::runtime_error("Failed to bind socket for free port lookup");
    }
    socklen_t len = sizeof(addr);
    if (getsockname(sock, reinterpret_cast<sockaddr*>(&addr), &len) != 0) { // NOLINT
        close(sock);
        throw std::runtime_error("Failed to read bound socket port");
    }
    int port = ntohs(addr.sin_port);
    close(sock);
    return port;
}

auto waitForServer(const std::string& listen_address) -> void {
    auto channel = grpc::CreateChannel(listen_address, grpc::InsecureChannelCredentials());
    gpr_timespec deadline = gpr_time_add(gpr_now(GPR_CLOCK_REALTIME), gpr_time_from_seconds(10, GPR_TIMESPAN)); // NOLINT
    if (!channel->WaitForConnected(deadline)) {
        throw std::runtime_error("Content service test server did not become ready: " + listen_address);
    }
}

auto writeJson(const fs::path& path, const json& value) -> void {
    std::ofstream output(path);
    if (!output) {
        throw std::runtime_error("Failed to write config file: " + path.string());
    }
    output << value.dump(4);
}

auto runClient(auto&& coro_factory) -> void {
    agrpc::GrpcContext client_context;
    std::exception_ptr error;
    boost::asio::co_spawn(client_context, std::forward<decltype(coro_factory)>(coro_factory),
                          [&error](std::exception_ptr ep) -> auto { error = std::move(ep); });
    client_context.run();
    if (error) {
        std::rethrow_exception(error);
    }
}

class ContentServiceClientIntegrationTest : public ::testing::Test {
  protected:
    static auto SetUpTestSuite() -> void {
        oink_judge::utils::grpc::registerChannelType();
        oink_judge::utils::grpc::registerInsecureCredentialsType();
        registerContentServiceChannelStubType();
    }

    auto SetUp() -> void override {
        const char* server_exe = std::getenv(SERVER_EXE_ENV);
        if (server_exe == nullptr || !fs::exists(server_exe)) {
            GTEST_SKIP() << SERVER_EXE_ENV << " is not set or executable was not built";
        }
        server_exe_ = server_exe;

        resources_ = fs::path("resources") / "test_content_service_server";
        root_ = fs::temp_directory_path() / "content_service_client_integration";
        fs::create_directories(root_);

        client_problems_dir_ = root_ / "client_problems";
        fs::create_directories(client_problems_dir_);

        server_problems_dir_ = resources_ / "problems";
        server_mut_dir_ = server_problems_dir_ / "mut";
        fs::create_directories(server_mut_dir_);
        oink_judge::utils::filesystem::storeFile(server_mut_dir_ / "existing.txt", "seed content");

        const int port = freePort();
        listen_address_ = "127.0.0.1:" + std::to_string(port);

        server_config_path_ = root_ / "server_config.json";
        writeJson(server_config_path_, json{{"directories", json{{"problems", server_problems_dir_.string()}}},
                                            {"timings", json{{"full_rescan_interval", 0.0}}}});

        credentials_path_ = root_ / "credentials.json";
        writeJson(credentials_path_, json::object());

        client_config_path_ = root_ / "client_config.json";
        writeJson(
            client_config_path_,
            json{{"directories", json{{"problems", client_problems_dir_.string()}}},
                 {"timings", json{{"full_rescan_interval", 0.0}}},
                 {"content_storage", json{{"stub_type", "content_service_stub(channel(" + listen_address_ + ",insecure))"}}}});

        Config::setConfigFilePath(server_config_path_);
        Config::setCredentialsFilePath(credentials_path_);
        Config::reloadData();

        server_pid_ = fork();
        if (server_pid_ == 0) {
            execl(server_exe_.c_str(), server_exe_.c_str(), server_config_path_.c_str(), credentials_path_.c_str(), // NOLINT
                  listen_address_.c_str(), nullptr);
            _exit(1);
        }
        if (server_pid_ < 0) {
            throw std::runtime_error("Failed to fork content service test server");
        }

        waitForServer(listen_address_);

        Config::setConfigFilePath(client_config_path_);
        Config::setCredentialsFilePath(credentials_path_);
        Config::reloadData();
    }

    auto TearDown() -> void override {
        if (server_pid_ > 0) {
            kill(server_pid_, SIGTERM);
            waitpid(server_pid_, nullptr, 0);
            server_pid_ = -1;
        }

        fs::remove(server_mut_dir_ / "uploaded.txt");
        fs::remove(server_mut_dir_ / "existing.txt");
        fs::remove_all(client_problems_dir_);
        fs::remove_all(root_);
    }

    static auto makeStub() -> std::unique_ptr<ContentServiceStub> {
        auto stub_opt = getContentStorageStub();
        if (!stub_opt.has_value()) {
            throw std::runtime_error("Failed to create ContentServiceStub from client config");
        }
        return std::move(*stub_opt);
    }

    fs::path resources_;           // NOLINT
    fs::path root_;                // NOLINT
    fs::path client_problems_dir_; // NOLINT
    fs::path server_problems_dir_; // NOLINT
    fs::path server_mut_dir_;      // NOLINT
    fs::path server_config_path_;  // NOLINT
    fs::path credentials_path_;    // NOLINT
    fs::path client_config_path_;  // NOLINT
    std::string server_exe_;       // NOLINT
    std::string listen_address_;   // NOLINT
    pid_t server_pid_ = -1;        // NOLINT
};

} // namespace

TEST_F(ContentServiceClientIntegrationTest, GetManifestViaConfiguredStub) {
    auto stub = makeStub();

    tl::expected<json, grpc::Status> manifest;
    runClient([&]() -> awaitable<void> { manifest = co_await stub->getManifest("problem", "1"); }); // NOLINT

    ASSERT_TRUE(manifest.has_value());
    ASSERT_TRUE(manifest->contains("files"));
    EXPECT_TRUE(manifest->at("files").contains("input.txt"));
    EXPECT_TRUE(manifest->at("files").contains("subdir/nested.txt"));
}

TEST_F(ContentServiceClientIntegrationTest, GetFileViaConfiguredStub) {
    auto stub = makeStub();
    const std::string expected = oink_judge::utils::filesystem::loadFile(resources_ / "problems" / "1" / "input.txt");

    tl::expected<std::string, grpc::Status> file;
    runClient([&]() -> awaitable<void> { file = co_await stub->getFile("problem", "1", "input.txt"); }); // NOLINT

    ASSERT_TRUE(file.has_value());
    EXPECT_EQ(*file, expected);
}

TEST_F(ContentServiceClientIntegrationTest, SyncContentDownloadsFromServer) {
    ContentStorage storage(makeStub());

    runClient([&]() -> awaitable<void> { co_await storage.syncContent("problem", "1"); }); // NOLINT

    const fs::path local_problem_dir = client_problems_dir_ / "1";
    const std::string expected_input = oink_judge::utils::filesystem::loadFile(resources_ / "problems" / "1" / "input.txt");
    const std::string expected_nested =
        oink_judge::utils::filesystem::loadFile(resources_ / "problems" / "1" / "subdir" / "nested.txt");

    EXPECT_EQ(oink_judge::utils::filesystem::loadFile(local_problem_dir / "input.txt"), expected_input);
    EXPECT_EQ(oink_judge::utils::filesystem::loadFile(local_problem_dir / "subdir" / "nested.txt"), expected_nested);
    EXPECT_TRUE(fs::exists(local_problem_dir / "manifest.json"));
}

TEST_F(ContentServiceClientIntegrationTest, UpdateContentOnServerUploadsNewFile) {
    const fs::path local_mut_dir = client_problems_dir_ / "mut";
    fs::create_directories(local_mut_dir);
    const std::string uploaded_content = "uploaded from c++ client";
    oink_judge::utils::filesystem::storeFile(local_mut_dir / "uploaded.txt", uploaded_content);

    ContentStorage storage(makeStub());
    runClient([&]() -> awaitable<void> { co_await storage.updateContentOnServer("problem", "mut"); }); // NOLINT

    const fs::path server_file = server_mut_dir_ / "uploaded.txt";
    EXPECT_TRUE(fs::exists(server_file));
    EXPECT_EQ(oink_judge::utils::filesystem::loadFile(server_file), uploaded_content);

    auto stub = makeStub();
    tl::expected<std::string, grpc::Status> file;
    runClient([&]() -> awaitable<void> { file = co_await stub->getFile("problem", "mut", "uploaded.txt"); }); // NOLINT
    ASSERT_TRUE(file.has_value());
    EXPECT_EQ(*file, uploaded_content);
}
