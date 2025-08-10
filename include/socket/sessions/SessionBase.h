#pragma once

#include "socket/Session.hpp"
#include "socket/SessionEventHandler.hpp"

namespace oink_judge::socket {

class SessionBase : public Session, public std::enable_shared_from_this<Session> {
public:
    ~SessionBase() override;
protected:
    SessionBase(tcp::socket socket, std::unique_ptr<SessionEventHandler> event_handler);

    SessionEventHandler &access_event_handler();
    tcp::socket &access_socket();

private:
    tcp::socket _socket;
    std::unique_ptr<SessionEventHandler> _event_handler;
};

} // namespace oink_judge::socket
