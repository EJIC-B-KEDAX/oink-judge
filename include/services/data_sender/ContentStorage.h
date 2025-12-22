#pragma once

#include <string>
#include "socket/Session.hpp"

namespace oink_judge::services::data_sender {

using Session = socket::Session;

class ContentStorage {
public:
    using CallbackFunc = std::function<void(std::error_code)>;

    static ContentStorage &instance();

    ContentStorage(const ContentStorage &) = delete;
    ContentStorage &operator=(const ContentStorage &) = delete;

    void ensure_content_exists(const std::string &content_type, const std::string &content_id, CallbackFunc callback);
    void update_content_on_server(const std::string &content_type, const std::string &content_id);

private:
    ContentStorage();

    std::shared_ptr<Session> _session;
};

} // namespace oink_judge::services::data_sender
