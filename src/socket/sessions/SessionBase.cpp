#include "socket/sessions/SessionBase.h"

namespace oink_judge::socket {


SessionBase::~SessionBase() {
    if (_socket.is_open()) {
        _socket.close();
        _protocol->close_session();
    }
}

SessionBase::SessionBase(tcp::socket socket, std::unique_ptr<Protocol> event_handler)
    : _socket(std::move(socket)), _protocol(std::move(event_handler)) {
}

Protocol &SessionBase::access_protocol() {
    if (!_protocol) {
        throw std::runtime_error("SessionEventHandler is not set.");
    }

    return *_protocol;
}

tcp::socket &SessionBase::access_socket() {
    return _socket;
}

} // namespace oink_judge::socket
