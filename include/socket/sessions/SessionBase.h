#pragma once

#include "socket/Session.hpp"
#include "socket/Protocol.hpp"

namespace oink_judge::socket {

class SessionBase : public Session, public std::enable_shared_from_this<Session> {
public:
    ~SessionBase() override;
protected:
    SessionBase(tcp::socket socket, std::unique_ptr<Protocol> event_handler);

    Protocol &access_protocol();
    tcp::socket &access_socket();

private:
    tcp::socket _socket;
    std::unique_ptr<Protocol> _protocol;
};

} // namespace oink_judge::socket
