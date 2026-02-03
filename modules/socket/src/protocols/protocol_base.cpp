#include "socket/protocols/ProtocolBase.h"


namespace oink_judge::socket {

awaitable<void> ProtocolBase::send_message(const std::string &message) {
    co_await get_session()->send_message(message);
}

void ProtocolBase::set_session(std::weak_ptr<socket::Session> session) {
    _session = session;
}

std::shared_ptr<socket::Session> ProtocolBase::get_session() const {
    return _session.lock();
}

void ProtocolBase::request_internal(const std::string &message, const callback_t &callback) {
    throw std::runtime_error("Request is not implemented for this protocol");
}

} // namespace oink_judge::socket
