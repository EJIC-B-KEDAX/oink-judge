#include "oink_judge/auth_service/protocol_with_fastapi.h"

#include "oink_judge/auth_service/handle_request.h"

namespace oink_judge::auth_service {

namespace {

[[maybe_unused]] const bool REGISTERED = []() -> bool {
    socket::ProtocolFactory::instance().registerType(
        ProtocolWithFastAPI::REGISTERED_NAME,
        [](const std::string& params) -> std::unique_ptr<socket::Protocol> { return std::make_unique<ProtocolWithFastAPI>(); });
    return true;
}();

} // namespace

ProtocolWithFastAPI::ProtocolWithFastAPI() = default;

auto ProtocolWithFastAPI::start(std::string start_message) -> awaitable<void> {
    co_spawn(co_await boost::asio::this_coro::executor, getSession()->receiveMessage(), boost::asio::detached);
}

auto ProtocolWithFastAPI::receiveMessage(std::string message) -> awaitable<void> {
    auto json_message = nlohmann::json::parse(message);
    co_await getSession()->sendMessage(handleClient(json_message).dump());
    co_spawn(co_await boost::asio::this_coro::executor, getSession()->receiveMessage(), boost::asio::detached);
}

auto ProtocolWithFastAPI::closeSession() -> void {}

} // namespace oink_judge::auth_service
