#include "socket/sessions/SessionBase.h"

namespace oink_judge::socket {

void SessionBase::request_internal(const std::string &message, const callback_t &callback) {
    access_protocol().request_internal(message, callback);
}

SessionBase::SessionBase(std::unique_ptr<Protocol> protocol) : _protocol(std::move(protocol)) {}

Protocol &SessionBase::access_protocol() {
    if (!_protocol) {
        throw std::runtime_error("SessionEventHandler is not set.");
    }

    return *_protocol;
}

} // namespace oink_judge::socket
