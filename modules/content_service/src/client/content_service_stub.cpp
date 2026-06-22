#include "oink_judge/content_service/client/content_service_stub.h"

#include "agrpc/grpc_context.hpp"
#include "oink_judge/factory/parameterized_type_factory.hpp"
#include "tl/expected.hpp"

#include <oink_judge/utils/grpc/factories.hpp>

#include <agrpc/client_rpc.hpp>
#include <boost/asio.hpp>
#include <grpcpp/client_context.h>
#include <grpcpp/grpcpp.h>

#include <string>

namespace oink_judge::content_service {

namespace {

using GetManifestRPC = agrpc::ClientRPC<&ContentService::Stub::PrepareAsyncGetManifest>;
using GetFileRPC = agrpc::ClientRPC<&ContentService::Stub::PrepareAsyncGetFile>;
using CreateFileRPC = agrpc::ClientRPC<&ContentService::Stub::PrepareAsyncCreateFile>;
using UpdateFileRPC = agrpc::ClientRPC<&ContentService::Stub::PrepareAsyncUpdateFile>;
using DeleteFileRPC = agrpc::ClientRPC<&ContentService::Stub::PrepareAsyncDeleteFile>;

} // namespace

auto ContentServiceChannelStub::getManifest(std::string content_type, std::string content_id)
    -> awaitable<tl::expected<json, grpc::Status>> {
    auto& grpc_context = static_cast<agrpc::GrpcContext&>((co_await boost::asio::this_coro::executor).context()); // NOLINT

    GetManifestRequest request;
    request.set_content_type(std::move(content_type));
    request.set_content_id(std::move(content_id));

    GetManifestResponse response;

    auto rpc = GetManifestRPC(grpc_context);
    co_await rpc.start(*stub_, request, boost::asio::use_awaitable);

    std::ostringstream manifest_stream;

    while (co_await rpc.read(response, boost::asio::use_awaitable)) {
        manifest_stream << response.manifest();
    }

    grpc::Status status = co_await rpc.finish(boost::asio::use_awaitable);

    if (!status.ok()) {
        co_return tl::unexpected(status);
    }
    json manifest_json = json::parse(manifest_stream.str());
    co_return manifest_json;
}

auto ContentServiceChannelStub::getFile(std::string content_type, std::string content_id, std::string file_path)
    -> awaitable<tl::expected<std::string, grpc::Status>> {
    auto& grpc_context = static_cast<agrpc::GrpcContext&>((co_await boost::asio::this_coro::executor).context()); // NOLINT

    GetFileRequest request;
    request.set_content_type(std::move(content_type));
    request.set_content_id(std::move(content_id));
    request.set_file_path(std::move(file_path));

    GetFileResponse response;

    auto rpc = GetFileRPC(grpc_context);
    co_await rpc.start(*stub_, request, boost::asio::use_awaitable);

    std::ostringstream file_content_stream(std::ios::binary);

    while (co_await rpc.read(response, boost::asio::use_awaitable)) {
        file_content_stream << response.file_chunk().data();
    }

    grpc::Status status = co_await rpc.finish(boost::asio::use_awaitable);

    if (!status.ok()) {
        co_return tl::unexpected(status);
    }
    co_return file_content_stream.str();
}

auto ContentServiceChannelStub::createFile(std::string content_type, std::string content_id, std::string file_path,
                                           std::string file_content) -> awaitable<tl::expected<void, grpc::Status>> {
    auto& grpc_context = static_cast<agrpc::GrpcContext&>((co_await boost::asio::this_coro::executor).context()); // NOLINT

    CreateFileRequest info;
    google::protobuf::Empty response;

    auto rpc = CreateFileRPC(grpc_context);

    auto* file_info = info.mutable_file_info();
    file_info->set_content_type(std::move(content_type));
    file_info->set_content_id(std::move(content_id));
    file_info->set_file_path(std::move(file_path));

    co_await rpc.start(*stub_, response, boost::asio::use_awaitable);

    co_await rpc.write(info, boost::asio::use_awaitable);

    constexpr auto CHUNK_SIZE = static_cast<size_t>(64 * 1024); // 64KB
    std::vector<char> buffer(CHUNK_SIZE);
    std::istringstream content_stream(std::move(file_content), std::ios::binary);

    while (content_stream) {
        content_stream.read(buffer.data(), static_cast<std::streamsize>(buffer.size()));
        std::streamsize bytes_read = content_stream.gcount();
        if (bytes_read == 0) {
            break;
        }
        if (bytes_read > 0) {
            CreateFileRequest chunk_request;
            auto* chunk = chunk_request.mutable_file_chunk();
            chunk->set_data(buffer.data(), static_cast<size_t>(bytes_read));
            co_await rpc.write(chunk_request, boost::asio::use_awaitable);
        }
    }

    grpc::Status status = co_await rpc.finish(boost::asio::use_awaitable);

    if (!status.ok()) {
        co_return tl::unexpected(status);
    }
    co_return tl::expected<void, grpc::Status>{};
}

auto ContentServiceChannelStub::updateFile(std::string content_type, std::string content_id, std::string file_path,
                                           std::string file_content) -> awaitable<tl::expected<void, grpc::Status>> {
    auto& grpc_context = static_cast<agrpc::GrpcContext&>((co_await boost::asio::this_coro::executor).context()); // NOLINT

    UpdateFileRequest info;
    google::protobuf::Empty response;

    auto* file_info = info.mutable_file_info();
    file_info->set_content_type(std::move(content_type));
    file_info->set_content_id(std::move(content_id));
    file_info->set_file_path(std::move(file_path));

    auto rpc = UpdateFileRPC(grpc_context);

    co_await rpc.start(*stub_, response, boost::asio::use_awaitable);

    co_await rpc.write(info, boost::asio::use_awaitable);

    constexpr auto CHUNK_SIZE = static_cast<size_t>(64 * 1024); // 64KB
    std::vector<char> buffer(CHUNK_SIZE);
    std::istringstream content_stream(std::move(file_content), std::ios::binary);

    while (content_stream) {
        content_stream.read(buffer.data(), static_cast<std::streamsize>(buffer.size()));
        std::streamsize bytes_read = content_stream.gcount();
        if (bytes_read == 0) {
            break;
        }
        if (bytes_read > 0) {
            UpdateFileRequest chunk_request;
            auto* chunk = chunk_request.mutable_file_chunk();
            chunk->set_data(buffer.data(), static_cast<size_t>(bytes_read));
            co_await rpc.write(chunk_request, boost::asio::use_awaitable);
        }
    }

    grpc::Status status = co_await rpc.finish(boost::asio::use_awaitable);

    if (!status.ok()) {
        co_return tl::unexpected(status);
    }
    co_return tl::expected<void, grpc::Status>{};
}

auto ContentServiceChannelStub::deleteFile(std::string content_type, std::string content_id, std::string file_path)
    -> awaitable<tl::expected<void, grpc::Status>> {
    auto& grpc_context = static_cast<agrpc::GrpcContext&>((co_await boost::asio::this_coro::executor).context()); // NOLINT

    DeleteFileRequest request;
    request.set_content_type(std::move(content_type));
    request.set_content_id(std::move(content_id));
    request.set_file_path(std::move(file_path));

    google::protobuf::Empty response;
    grpc::ClientContext context;
    grpc::Status status =
        co_await DeleteFileRPC::request(grpc_context, *stub_, context, request, response, boost::asio::use_awaitable);

    if (!status.ok()) {
        co_return tl::unexpected(status);
    }
    co_return tl::expected<void, grpc::Status>{};
}

ContentServiceChannelStub::ContentServiceChannelStub(std::shared_ptr<grpc::Channel> channel) : channel_(std::move(channel)) {
    stub_ = ContentService::NewStub(channel_);
}

auto registerContentServiceChannelStubType() -> void {
    ContentServiceStubFactory::instance().registerType(
        ContentServiceChannelStub::REGISTERED_NAME, [](const std::string& param) -> std::unique_ptr<ContentServiceChannelStub> {
            std::vector<std::string> parts = factory::parseParameters(param, ",");
            if (parts.size() != 1) {
                throw std::invalid_argument("Expected exactly one parameter for channel_stub: channel type");
            }
            std::string channel_type = factory::normalizeArgument(parts[0], true);
            auto channel = utils::grpc::ChannelFactory::instance().create(channel_type);
            return std::make_unique<ContentServiceChannelStub>(std::move(channel));
        });
}

} // namespace oink_judge::content_service
