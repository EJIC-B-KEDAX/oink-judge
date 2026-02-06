#include "oink_judge/socket/protocols/ponging_protocol.h"

#include <oink_judge/config/config.h>

namespace oink_judge::socket {

namespace {

[[maybe_unused]] const bool REGISTERED = []() -> bool {
    ProtocolFactory::instance().registerType(
        PongingProtocol::REGISTERED_NAME, [](const std::string& params) -> std::unique_ptr<Protocol> {
            const std::string& inner_type = params;

            return std::make_unique<PongingProtocol>(ProtocolFactory::instance().create(inner_type));
        });
    return true;
}();

} // namespace

PongingProtocol::PongingProtocol(std::unique_ptr<Protocol> inner_protocol) : ProtocolDecorator(std::move(inner_protocol)) {}

auto PongingProtocol::receiveMessage(std::string message) -> awaitable<void> {
    if (message == "ping") {
        boost::asio::co_spawn(co_await boost::asio::this_coro::executor, getSession()->receiveMessage(), boost::asio::detached);
        co_await sendMessage("pong");
        co_return;
    }

    co_await ProtocolDecorator::receiveMessage(message);
}

} // namespace oink_judge::socket
