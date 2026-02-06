#include "oink_judge/socket/sessions/session_base.h"

#include <oink_judge/logger/logger.h>
namespace oink_judge::socket {

auto SessionBase::setSessionPtr() -> void { protocol_->setSession(shared_from_this()); }

auto SessionBase::requestInternal(const std::string& message, const callback_t& callback) -> void {
    accessProtocol().requestInternal(message, callback);
}

SessionBase::SessionBase(std::unique_ptr<Protocol> protocol) : protocol_(std::move(protocol)) {}

auto SessionBase::accessProtocol() -> Protocol& {
    if (!protocol_) {
        logger::logMessage("socket", 1, "Protocol is not set for session", logger::ERROR);
        throw std::runtime_error("Protocol is not set.");
    }

    return *protocol_;
}

} // namespace oink_judge::socket
