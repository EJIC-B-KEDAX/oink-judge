#include "socket/protocols/PongingProtocol.h"
#include "config/Config.h"

namespace oink_judge::socket {

namespace {

[[maybe_unused]] bool registered = []() {
    ProtocolFactory::instance().register_type(PongingProtocol::REGISTERED_NAME,
        [](const std::string &params) -> std::unique_ptr<Protocol> {
        std::string inner_type = params;

        return std::make_unique<PongingProtocol>(ProtocolFactory::instance().create(inner_type));
    });
    return true;
}();

} // namespace

PongingProtocol::PongingProtocol(std::unique_ptr<Protocol> inner_protocol)
    : _inner_protocol(std::move(inner_protocol)) {}

void PongingProtocol::start(const std::string &start_message) {
    _inner_protocol->start(start_message);
}

void PongingProtocol::receive_message(const std::string &message) {
    if (message == "ping") {
        get_session()->send_message("pong");
        get_session()->receive_message();
        return;
    }

    _inner_protocol->receive_message(message);
}

void PongingProtocol::close_session() {
    _inner_protocol->close_session();
}

void PongingProtocol::set_session(std::weak_ptr<Session> session) {
    _inner_protocol->set_session(session);
}

std::shared_ptr<Session> PongingProtocol::get_session() const {
    return _inner_protocol->get_session();
}

void PongingProtocol::request_internal(const std::string &message, const callback_t &callback) {
    _inner_protocol->request_internal(message, callback);
}

} // namespace oink_judge::socket
