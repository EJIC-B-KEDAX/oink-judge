#pragma once
#include "content_service_stub.h"

#include <boost/asio/awaitable.hpp>
#include <nlohmann/json.hpp>

#include <string>
#include <vector>

namespace oink_judge::content_service {

using boost::asio::awaitable;
using nlohmann::json;

class ContentStorage {
  public:
    static auto instance() -> ContentStorage&;

    explicit ContentStorage(std::unique_ptr<ContentServiceStub> stub);

    ContentStorage(const ContentStorage&) = delete;
    auto operator=(const ContentStorage&) -> ContentStorage& = delete;
    ContentStorage(ContentStorage&&) = delete;
    auto operator=(ContentStorage&&) -> ContentStorage& = delete;
    ~ContentStorage() = default;

    auto syncContent(std::string content_type, std::string content_id) -> awaitable<void>;
    auto syncFile(std::string content_type, std::string content_id, std::string file_path) -> awaitable<void>;
    auto updateContentOnServer(std::string content_type, std::string content_id) -> awaitable<void>;
    auto createContent(std::string content_type, std::string content_id) -> awaitable<void>;
    auto listContent(std::string content_type) -> awaitable<std::vector<std::string>>;

  private:
    ContentStorage();

    auto getManifestFromServer(std::string content_type, std::string content_id) -> awaitable<json>;

    auto getFileFromServer(std::string content_type, std::string content_id, std::string file_path) -> awaitable<std::string>;

    auto createFileOnServer(std::string content_type, std::string content_id, std::string file_path, std::string file_content)
        -> awaitable<void>;

    auto updateFileOnServer(std::string content_type, std::string content_id, std::string file_path, std::string file_content)
        -> awaitable<void>;

    auto removeFileOnServer(std::string content_type, std::string content_id, std::string file_path) -> awaitable<void>;

    auto setPermissionsOnServer(std::string content_type, std::string content_id, std::string file_path, uint32_t permissions)
        -> awaitable<void>;

    auto createContentOnServer(std::string content_type, std::string content_id) -> awaitable<void>;

    auto listContentOnServer(std::string content_type) -> awaitable<std::vector<std::string>>;

    std::unique_ptr<ContentServiceStub> stub_;
};

} // namespace oink_judge::content_service
