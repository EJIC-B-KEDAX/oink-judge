#pragma once

#include "socket/sessions/SessionBase.h"
#include <queue>

namespace oink_judge::socket {

class BasicSession : public SessionBase {
public:
    BasicSession(tcp::socket socket, std::unique_ptr<SessionEventHandler> event_handler);

    void start(const std::string &start_message) override;
    void send_message(const std::string &message) override;
    void receive_message() override;
    void close() override;

    constexpr static auto REGISTERED_NAME = "BasicSession";
private:
    void _send_next();

    std::queue<std::string> _message_queue;
};

} // namespace oink_judge::socket
