#pragma once
#include "socket/Session.hpp"

#include <nlohmann/json.hpp>
#include <string>

namespace oink_judge::services::data_sender {

using boost::asio::awaitable;
using nlohmann::json;
using socket::Session;

class ContentStorage {
  public:
    static ContentStorage& instance();

    ContentStorage(const ContentStorage&) = delete;
    ContentStorage& operator=(const ContentStorage&) = delete;

    awaitable<void> ensure_content_exists(const std::string& content_type, const std::string& content_id);
    awaitable<void> update_content_on_server(const std::string& content_type, const std::string& content_id);

  private:
    ContentStorage();

    std::shared_ptr<Session> _session;

    awaitable<json> get_manifest_from_server(const std::string& content_type, const std::string& content_id);

    awaitable<std::string> get_file_from_server(const std::string& content_type, const std::string& content_id,
                                                const std::string& file_path);

    awaitable<void> update_file_on_server(const std::string& content_type, const std::string& content_id,
                                          const std::string& file_path, const std::string& file_content);

    awaitable<void> remove_file_on_server(const std::string& content_type, const std::string& content_id,
                                          const std::string& file_path);
};

} // namespace oink_judge::services::data_sender
