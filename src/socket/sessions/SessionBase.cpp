#include "socket/sessions/SessionBase.h"

namespace oink_judge::socket {


SessionBase::~SessionBase() {
    if (_socket.is_open()) {
        _socket.close();
        _event_handler->close_session();
    }
}

SessionBase::SessionBase(tcp::socket socket, std::unique_ptr<SessionEventHandler> event_handler)
    : _socket(std::move(socket)), _event_handler(std::move(event_handler)) {
}

SessionEventHandler &SessionBase::access_event_handler() {
    if (!_event_handler) {
        throw std::runtime_error("SessionEventHandler is not set.");
    }

    return *_event_handler;
}

tcp::socket &SessionBase::access_socket() {
    return _socket;
}

} // namespace oink_judge::socket
