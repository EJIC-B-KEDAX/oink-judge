#pragma once
#include "oink_judge/content_service.grpc.pb.h"

#include <oink_judge/factory/parameterized_type_factory.hpp>

#include <boost/asio/awaitable.hpp>
#include <nlohmann/json.hpp>
#include <tl/expected.hpp>

#include <memory>

namespace oink_judge::content_service {

using boost::asio::awaitable;
using nlohmann::json;

class ContentServiceStub {
  public:
    ContentServiceStub() = default;
    ContentServiceStub(const ContentServiceStub&) = delete;
    auto operator=(const ContentServiceStub&) -> ContentServiceStub& = delete;
    ContentServiceStub(ContentServiceStub&&) = delete;
    auto operator=(ContentServiceStub&&) -> ContentServiceStub& = delete;
    virtual ~ContentServiceStub() = default;

    virtual auto getManifest(std::string content_type, std::string content_id) -> awaitable<tl::expected<json, grpc::Status>> = 0;
    virtual auto getFile(std::string content_type, std::string content_id, std::string file_path)
        -> awaitable<tl::expected<std::string, grpc::Status>> = 0;
    virtual auto createFile(std::string content_type, std::string content_id, std::string file_path, std::string file_content)
        -> awaitable<tl::expected<void, grpc::Status>> = 0;
    virtual auto updateFile(std::string content_type, std::string content_id, std::string file_path, std::string file_content)
        -> awaitable<tl::expected<void, grpc::Status>> = 0;
    virtual auto deleteFile(std::string content_type, std::string content_id, std::string file_path)
        -> awaitable<tl::expected<void, grpc::Status>> = 0;
};

class ContentServiceChannelStub : public ContentServiceStub {
  public:
    ContentServiceChannelStub(std::shared_ptr<grpc::Channel> channel);

    auto getManifest(std::string content_type, std::string content_id) -> awaitable<tl::expected<json, grpc::Status>> override;
    auto getFile(std::string content_type, std::string content_id, std::string file_path)
        -> awaitable<tl::expected<std::string, grpc::Status>> override;
    auto createFile(std::string content_type, std::string content_id, std::string file_path, std::string file_content)
        -> awaitable<tl::expected<void, grpc::Status>> override;
    auto updateFile(std::string content_type, std::string content_id, std::string file_path, std::string file_content)
        -> awaitable<tl::expected<void, grpc::Status>> override;
    auto deleteFile(std::string content_type, std::string content_id, std::string file_path)
        -> awaitable<tl::expected<void, grpc::Status>> override;

    constexpr static auto REGISTERED_NAME = "content_service_stub";

  private:
    std::shared_ptr<grpc::Channel> channel_;
    std::unique_ptr<ContentService::Stub> stub_;
};

auto registerContentServiceChannelStubType() -> void;

using ContentServiceStubFactory = factory::ParameterizedTypeFactory<std::unique_ptr<ContentServiceStub>>;

} // namespace oink_judge::content_service
