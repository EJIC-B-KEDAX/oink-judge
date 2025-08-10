#pragma once

#include <string>
#include "socket/SessionEventHandler.hpp"

namespace oink_judge::services::data_sender {

using Session = socket::Session;

class StorageSessionEventHandler : public socket::SessionEventHandler {
public:
    StorageSessionEventHandler();

    void start(const std::string &start_message) override;
    void receive_message(const std::string &message) override;
    void close_session() override;

    void set_session(std::weak_ptr<socket::Session> session) override;
    std::weak_ptr<socket::Session> get_session() const override;

    constexpr static auto REGISTERED_NAME = "StorageSessionEventHandler";
    
private:
    std::weak_ptr<socket::Session> _session;

    enum Status {
        WAIT_HEADER,
        WAIT_DATA
    };
    Status status;
    std::string content_type;
    std::string content_id;
};

} // namespace oink_judge::services::data_sender
