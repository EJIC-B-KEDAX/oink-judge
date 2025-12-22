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
    : ProtocolDecorator(std::move(inner_protocol)) {}


void PongingProtocol::receive_message(const std::string &message) {
    if (message == "ping") {
        send_message("pong");
        get_session()->receive_message();
        return;
    }

    ProtocolDecorator::receive_message(message);
}

} // namespace oink_judge::socket
