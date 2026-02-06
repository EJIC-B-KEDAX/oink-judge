#include "oink_judge/socket/protocols/protocol_base.h"

#include <oink_judge/logger/logger.h>

namespace oink_judge::socket {

auto ProtocolBase::sendMessage(std::string message) -> awaitable<void> { co_await getSession()->sendMessage(std::move(message)); }

auto ProtocolBase::setSession(std::weak_ptr<socket::Session> session) -> void { session_ = session; }

auto ProtocolBase::getSession() const -> std::shared_ptr<socket::Session> {
    if (session_.expired()) {
        logger::logMessage("socket", 1, "Trying to get session from protocol, but session is expired", logger::ERROR);
        throw std::runtime_error("Session is expired");
    }
    return session_.lock();
}

auto ProtocolBase::requestInternal(const std::string& message, const callback_t& callback) -> void {
    logger::logMessage("socket", 1, "Trying to request on base protocol, which is not implemented", logger::ERROR);
    throw std::runtime_error("Request is not implemented for this protocol");
}

} // namespace oink_judge::socket
