#include "socket/protocols/ProtocolDecorator.h"

namespace oink_judge::socket {

ProtocolDecorator::ProtocolDecorator(std::unique_ptr<Protocol> inner_protocol)
    : _inner_protocol(std::move(inner_protocol)) {}

void ProtocolDecorator::start(const std::string &start_message) {
    _inner_protocol->start(start_message);
}

void ProtocolDecorator::send_message(const std::string &message) {
    _inner_protocol->send_message(message);
}

void ProtocolDecorator::receive_message(const std::string &message) {
    _inner_protocol->receive_message(message);
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
