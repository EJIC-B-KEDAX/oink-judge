#pragma once

#include "socket/Session.hpp"
#include "socket/Protocol.hpp"

namespace oink_judge::socket {

class SessionBase : public Session, public std::enable_shared_from_this<Session> {
public:
    void set_session_ptr();
    void request_internal(const std::string &message, const callback_t &callback) override;

protected:
    SessionBase(std::unique_ptr<Protocol> protocol);

    Protocol &access_protocol();

private:
    std::unique_ptr<Protocol> _protocol;
};

} // namespace oink_judge::socket
