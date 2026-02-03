#include "socket/protocols/ProtocolDecorator.h"
#include <iostream>

namespace oink_judge::socket {

ProtocolDecorator::ProtocolDecorator(std::unique_ptr<Protocol> inner_protocol)
    : _inner_protocol(std::move(inner_protocol)) {}

awaitable<void> ProtocolDecorator::start(const std::string &start_message) {
    co_await _inner_protocol->start(start_message);
}

awaitable<void> ProtocolDecorator::send_message(const std::string &message) {
    std::cerr << "ProtocolDecorator sending message: " << message << std::endl;
    co_await _inner_protocol->send_message(message);
}

awaitable<void> ProtocolDecorator::receive_message(const std::string &message) {
    co_await _inner_protocol->receive_message(message);
}

void ProtocolDecorator::close_session() {
    _inner_protocol->close_session();
}

void ProtocolDecorator::set_session(std::weak_ptr<Session> session) {
    _inner_protocol->set_session(session);
}

std::shared_ptr<Session> ProtocolDecorator::get_session() const {
    return _inner_protocol->get_session();
}

void ProtocolDecorator::request_internal(const std::string &message, const callback_t &callback) {
    _inner_protocol->request_internal(message, callback);
}

std::unique_ptr<Protocol> &ProtocolDecorator::access_inner_protocol() {
    return _inner_protocol;
}

} // namespace oink_judge::socket
