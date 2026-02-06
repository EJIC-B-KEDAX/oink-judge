#include "oink_judge/socket/protocols/protocol_decorator.h"

namespace oink_judge::socket {

ProtocolDecorator::ProtocolDecorator(std::unique_ptr<Protocol> inner_protocol) : inner_protocol_(std::move(inner_protocol)) {}

auto ProtocolDecorator::start(std::string start_message) -> awaitable<void> { co_await inner_protocol_->start(start_message); }

auto ProtocolDecorator::sendMessage(std::string message) -> awaitable<void> { co_await inner_protocol_->sendMessage(message); }

auto ProtocolDecorator::receiveMessage(std::string message) -> awaitable<void> {
    co_await inner_protocol_->receiveMessage(message);
}

auto ProtocolDecorator::closeSession() -> void { inner_protocol_->closeSession(); }

auto ProtocolDecorator::setSession(std::weak_ptr<Session> session) -> void { inner_protocol_->setSession(session); }

auto ProtocolDecorator::getSession() const -> std::shared_ptr<Session> { return inner_protocol_->getSession(); }

auto ProtocolDecorator::requestInternal(const std::string& message, const callback_t& callback) -> void {
    inner_protocol_->requestInternal(message, callback);
}

auto ProtocolDecorator::accessInnerProtocol() -> std::unique_ptr<Protocol>& { return inner_protocol_; }

} // namespace oink_judge::socket
