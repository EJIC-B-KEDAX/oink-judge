#include "oink_judge/content_service/client/content_service_stub.h"
#include "oink_judge/content_service/content_manifest.h"
#include "oink_judge/content_service/server/content_service.h"

#include <oink_judge/config/config.h>
#include <oink_judge/utils/filesystem.h>

#include <agrpc/grpc_context.hpp>
#include <agrpc/register_awaitable_rpc_handler.hpp>
#include <boost/asio.hpp>
#include <grpcpp/grpcpp.h>
#include <gtest/gtest.h>

#include <filesystem>
#include <fstream>
#include <memory>
#include <string>
#include <thread>

using namespace oink_judge::content_service;
using nlohmann::json;
using oink_judge::config::Config;
namespace fs = std::filesystem;

namespace {

constexpr size_t CHUNK_SIZE = 64 * 1024; // NOLINT

auto makeBinaryContent(size_t size) -> std::string {
    std::string content(size, '\0');
    for (size_t i = 0; i < size; ++i) {
        content[i] = static_cast<char>(((i * 131) + 7) % 256); // NOLINT
    }
    return content;
}

} // namespace

// ---------------------------------------------------------------------------
// Fixture — in-process gRPC server with the real handlers, talked to through
// the real ContentServiceChannelStub over an insecure localhost channel.
// ---------------------------------------------------------------------------

class ContentServiceServerTest : public ::testing::Test {
  protected:
    auto SetUp() -> void override {
        resources_ = fs::path("resources") / "test_content_service_server";
        Config::setConfigFilePath(resources_ / "good_config.json");
        Config::setCredentialsFilePath(resources_ / "good_credentials.json");
        Config::reloadData();

        // Mutable content area recreated for every test
        fs::create_directories(resources_ / "problems" / "mut");
        oink_judge::utils::filesystem::storeFile(resources_ / "problems" / "mut" / "existing.txt", "seed content");

        int port = 0;
        grpc::ServerBuilder builder;
        builder.AddListeningPort("127.0.0.1:0", grpc::InsecureServerCredentials(), &port);
        builder.RegisterService(&service_);
        server_context_ = std::make_unique<agrpc::GrpcContext>(builder.AddCompletionQueue());
        server_ = builder.BuildAndStart();
        ASSERT_NE(server_, nullptr);
        ASSERT_NE(port, 0);

        agrpc::register_awaitable_rpc_handler<GetManifestRPC>(*server_context_, service_, &getManifestHandler,
                                                              boost::asio::detached);
        agrpc::register_awaitable_rpc_handler<SetPermissionsRPC>(*server_context_, service_, &setPermissionsHandler,
                                                                 boost::asio::detached);
        agrpc::register_awaitable_rpc_handler<GetFileRPC>(*server_context_, service_, &getFileHandler, boost::asio::detached);
        agrpc::register_awaitable_rpc_handler<CreateFileRPC>(*server_context_, service_, &createFileHandler,
                                                             boost::asio::detached);
        agrpc::register_awaitable_rpc_handler<UpdateFileRPC>(*server_context_, service_, &updateFileHandler,
                                                             boost::asio::detached);
        agrpc::register_awaitable_rpc_handler<DeleteFileRPC>(*server_context_, service_, &deleteFileHandler,
                                                             boost::asio::detached);
        agrpc::register_awaitable_rpc_handler<CreateContentRPC>(*server_context_, service_, &createContentHandler,
                                                                boost::asio::detached);
        agrpc::register_awaitable_rpc_handler<ListContentRPC>(*server_context_, service_, &listContentHandler,
                                                              boost::asio::detached);

        server_thread_ = std::thread([this] { server_context_->run(); });

        channel_ = grpc::CreateChannel("127.0.0.1:" + std::to_string(port), grpc::InsecureChannelCredentials());
        stub_ = std::make_unique<ContentServiceChannelStub>(channel_);
    }

    auto TearDown() -> void override {
        server_->Shutdown();
        server_thread_.join();
        server_context_.reset();
        server_.reset();

        fs::remove_all(resources_ / "problems" / "mut");
        fs::remove_all(resources_ / "problems" / "big_manifest");
        fs::remove(resources_ / "problems" / "1" / "manifest.json");
        fs::remove(resources_ / "problems" / "1" / "big.bin");
    }

    auto getResourcesPath() -> const fs::path& { return resources_; }

    auto stub() -> ContentServiceStub& { return *stub_; }

    auto getChannel() -> std::shared_ptr<grpc::Channel> { return channel_; }

    // Runs a client coroutine to completion on a dedicated client GrpcContext.
    template <typename CoroFactory> auto runClient(CoroFactory&& coro_factory) -> void {
        agrpc::GrpcContext client_context;
        std::exception_ptr error;
        boost::asio::co_spawn(client_context, std::forward<CoroFactory>(coro_factory),
                              [&error](std::exception_ptr ep) { error = std::move(ep); });
        client_context.run();
        if (error) {
            std::rethrow_exception(error);
        }
    }

  private:
    fs::path resources_;

    ContentService::AsyncService service_;
    std::unique_ptr<grpc::Server> server_;
    std::unique_ptr<agrpc::GrpcContext> server_context_;
    std::thread server_thread_;

    std::shared_ptr<grpc::Channel> channel_;
    std::unique_ptr<ContentServiceChannelStub> stub_;
};

// ---------------------------------------------------------------------------
// Request validation — path traversal and malformed arguments
// ---------------------------------------------------------------------------

TEST_F(ContentServiceServerTest, GetFileRejectsParentDirectoryTraversal) {
    tl::expected<std::string, grpc::Status> result;
    runClient([&]() -> awaitable<void> { result = co_await stub().getFile("problem", "1", "../2/data.txt"); }); // NOLINT

    ASSERT_FALSE(result.has_value());
    EXPECT_EQ(result.error().error_code(), grpc::StatusCode::INVALID_ARGUMENT);
}

TEST_F(ContentServiceServerTest, GetFileRejectsAbsolutePath) {
    tl::expected<std::string, grpc::Status> result;
    runClient([&]() -> awaitable<void> { result = co_await stub().getFile("problem", "1", "/etc/passwd"); }); // NOLINT

    ASSERT_FALSE(result.has_value());
    EXPECT_EQ(result.error().error_code(), grpc::StatusCode::INVALID_ARGUMENT);
}

TEST_F(ContentServiceServerTest, GetFileRejectsEmptyArguments) {
    tl::expected<std::string, grpc::Status> no_type;
    tl::expected<std::string, grpc::Status> no_id;
    tl::expected<std::string, grpc::Status> no_path;
    runClient([&]() -> awaitable<void> { // NOLINT
        no_type = co_await stub().getFile("", "1", "input.txt");
        no_id = co_await stub().getFile("problem", "", "input.txt");
        no_path = co_await stub().getFile("problem", "1", "");
    });

    ASSERT_FALSE(no_type.has_value());
    EXPECT_EQ(no_type.error().error_code(), grpc::StatusCode::INVALID_ARGUMENT);
    ASSERT_FALSE(no_id.has_value());
    EXPECT_EQ(no_id.error().error_code(), grpc::StatusCode::INVALID_ARGUMENT);
    ASSERT_FALSE(no_path.has_value());
    EXPECT_EQ(no_path.error().error_code(), grpc::StatusCode::INVALID_ARGUMENT);
}

TEST_F(ContentServiceServerTest, UnknownContentOrFileReturnsNotFound) {
    tl::expected<std::string, grpc::Status> no_content;
    tl::expected<std::string, grpc::Status> no_file;
    runClient([&]() -> awaitable<void> { // NOLINT
        no_content = co_await stub().getFile("problem", "nonexistent_id", "input.txt");
        no_file = co_await stub().getFile("problem", "1", "missing.txt");
    });

    ASSERT_FALSE(no_content.has_value());
    EXPECT_EQ(no_content.error().error_code(), grpc::StatusCode::NOT_FOUND);
    ASSERT_FALSE(no_file.has_value());
    EXPECT_EQ(no_file.error().error_code(), grpc::StatusCode::NOT_FOUND);
}

TEST_F(ContentServiceServerTest, GetFileOnDirectoryReturnsInvalidArgument) {
    tl::expected<std::string, grpc::Status> result;
    runClient([&]() -> awaitable<void> { result = co_await stub().getFile("problem", "1", "subdir"); }); // NOLINT

    ASSERT_FALSE(result.has_value());
    EXPECT_EQ(result.error().error_code(), grpc::StatusCode::INVALID_ARGUMENT);
}

TEST_F(ContentServiceServerTest, GetManifestValidatesArguments) {
    tl::expected<json, grpc::Status> empty_args;
    tl::expected<json, grpc::Status> missing_content;
    runClient([&]() -> awaitable<void> { // NOLINT
        empty_args = co_await stub().getManifest("", "");
        missing_content = co_await stub().getManifest("problem", "nonexistent_id");
    });

    ASSERT_FALSE(empty_args.has_value());
    EXPECT_EQ(empty_args.error().error_code(), grpc::StatusCode::INVALID_ARGUMENT);
    ASSERT_FALSE(missing_content.has_value());
    EXPECT_EQ(missing_content.error().error_code(), grpc::StatusCode::NOT_FOUND);
}

// ---------------------------------------------------------------------------
// Streaming round-trips
// ---------------------------------------------------------------------------

TEST_F(ContentServiceServerTest, GetFileStreamsLargeBinaryFileByteForByte) {
    std::string expected = makeBinaryContent((3 * CHUNK_SIZE) + 123); // NOLINT
    oink_judge::utils::filesystem::storeFile(getResourcesPath() / "problems" / "1" / "big.bin", expected);

    tl::expected<std::string, grpc::Status> result;
    runClient([&]() -> awaitable<void> { result = co_await stub().getFile("problem", "1", "big.bin"); }); // NOLINT

    ASSERT_TRUE(result.has_value());
    EXPECT_EQ(*result, expected);
}

TEST_F(ContentServiceServerTest, GetManifestStreamsLargeManifestAsValidJson) {
    // Enough files that the manifest exceeds one 64KB chunk
    constexpr int FILES_COUNT = 500;
    fs::path content_dir = getResourcesPath() / "problems" / "big_manifest";
    fs::create_directories(content_dir);
    for (int i = 0; i < FILES_COUNT; ++i) {
        std::ofstream file(content_dir / ("file_" + std::to_string(i) + "_padding_to_make_the_entry_longer.txt"));
        file << "content " << i;
    }

    tl::expected<json, grpc::Status> result;
    runClient([&]() -> awaitable<void> { result = co_await stub().getManifest("problem", "big_manifest"); }); // NOLINT

    ASSERT_TRUE(result.has_value());
    ASSERT_GT(result->dump(4).size(), CHUNK_SIZE); // chunking was actually exercised
    ASSERT_TRUE(result->contains("files"));
    EXPECT_EQ((*result)["files"].size(), static_cast<size_t>(FILES_COUNT));
    EXPECT_EQ(*result, ContentManifest("problem", "big_manifest").toJson());
}

TEST_F(ContentServiceServerTest, CreateFileFirstMessageWithoutFileInfoIsRejected) {
    // The real client stub always sends file_info first, so use the raw
    // generated stub to violate the protocol.
    auto raw_stub = ContentService::NewStub(getChannel());
    grpc::ClientContext context;
    google::protobuf::Empty response;
    auto writer = raw_stub->CreateFile(&context, &response);

    CreateFileRequest chunk_request;
    chunk_request.mutable_file_chunk()->set_data("violates protocol");
    writer->Write(chunk_request);
    writer->WritesDone();
    grpc::Status status = writer->Finish();

    EXPECT_EQ(status.error_code(), grpc::StatusCode::INVALID_ARGUMENT);
}

TEST_F(ContentServiceServerTest, CreateFileOnExistingFileReturnsAlreadyExists) {
    tl::expected<void, grpc::Status> result;
    runClient([&]() -> awaitable<void> { // NOLINT
        result = co_await stub().createFile("problem", "mut", "existing.txt", "overwrite attempt");
    });

    ASSERT_FALSE(result.has_value());
    EXPECT_EQ(result.error().error_code(), grpc::StatusCode::ALREADY_EXISTS);
    EXPECT_EQ(oink_judge::utils::filesystem::loadFile(getResourcesPath() / "problems" / "mut" / "existing.txt"), "seed content");
}

TEST_F(ContentServiceServerTest, CreateFileMultiChunkUploadWritesNestedFile) {
    std::string content = makeBinaryContent((2 * CHUNK_SIZE) + 17); // NOLINT

    tl::expected<void, grpc::Status> result;
    runClient(
        [&]() -> awaitable<void> { result = co_await stub().createFile("problem", "mut", "sub/new.bin", content); }); // NOLINT

    ASSERT_TRUE(result.has_value());
    EXPECT_EQ(oink_judge::utils::filesystem::loadFile(getResourcesPath() / "problems" / "mut" / "sub" / "new.bin"), content);
}

TEST_F(ContentServiceServerTest, UpdateFileOnMissingFileReturnsNotFound) {
    tl::expected<void, grpc::Status> result;
    runClient(
        [&]() -> awaitable<void> { result = co_await stub().updateFile("problem", "mut", "missing.txt", "content"); }); // NOLINT

    ASSERT_FALSE(result.has_value());
    EXPECT_EQ(result.error().error_code(), grpc::StatusCode::NOT_FOUND);
}

TEST_F(ContentServiceServerTest, UpdateFileOverwritesContentIncludingTruncation) {
    fs::path file_path = getResourcesPath() / "problems" / "mut" / "existing.txt";

    tl::expected<void, grpc::Status> longer;
    tl::expected<void, grpc::Status> shorter;
    runClient([&]() -> awaitable<void> { // NOLINT
        longer = co_await stub().updateFile("problem", "mut", "existing.txt", "a much longer replacement content");
    });
    ASSERT_TRUE(longer.has_value());
    EXPECT_EQ(oink_judge::utils::filesystem::loadFile(file_path), "a much longer replacement content");

    runClient(
        [&]() -> awaitable<void> { shorter = co_await stub().updateFile("problem", "mut", "existing.txt", "x"); }); // NOLINT
    ASSERT_TRUE(shorter.has_value());
    EXPECT_EQ(oink_judge::utils::filesystem::loadFile(file_path), "x");
}

TEST_F(ContentServiceServerTest, DeleteFileRemovesFileFromDisk) {
    fs::path file_path = getResourcesPath() / "problems" / "mut" / "existing.txt";
    ASSERT_TRUE(fs::exists(file_path));

    tl::expected<void, grpc::Status> delete_result;
    tl::expected<std::string, grpc::Status> get_after_delete;
    runClient([&]() -> awaitable<void> { // NOLINT
        delete_result = co_await stub().deleteFile("problem", "mut", "existing.txt");
        get_after_delete = co_await stub().getFile("problem", "mut", "existing.txt");
    });

    ASSERT_TRUE(delete_result.has_value());
    EXPECT_FALSE(fs::exists(file_path));
    ASSERT_FALSE(get_after_delete.has_value());
    EXPECT_EQ(get_after_delete.error().error_code(), grpc::StatusCode::NOT_FOUND);
}
