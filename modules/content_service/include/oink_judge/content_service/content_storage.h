#pragma once
#include <nlohmann/json.hpp>
#include <oink_judge/socket/session.hpp>
#include <string>

namespace oink_judge::content_service {

using boost::asio::awaitable;
using nlohmann::json;
using socket::Session;

class ContentStorage {
  public:
    static auto instance() -> ContentStorage&;

    ContentStorage(const ContentStorage&) = delete;
    auto operator=(const ContentStorage&) -> ContentStorage& = delete;
    ContentStorage(ContentStorage&&) = delete;
    auto operator=(ContentStorage&&) -> ContentStorage& = delete;
    ~ContentStorage() = default;

    auto ensureContentExists(std::string content_type, std::string content_id) -> awaitable<void>;
    auto updateContentOnServer(std::string content_type, std::string content_id) -> awaitable<void>;

  private:
    ContentStorage();

    std::shared_ptr<Session> session_;

    auto getManifestFromServer(std::string content_type, std::string content_id) -> awaitable<json>;

    auto getFileFromServer(std::string content_type, std::string content_id, std::string file_path) -> awaitable<std::string>;

    auto updateFileOnServer(std::string content_type, std::string content_id, std::string file_path, std::string file_content)
        -> awaitable<void>;

    auto removeFileOnServer(std::string content_type, std::string content_id, std::string file_path) -> awaitable<void>;
};

} // namespace oink_judge::content_service
