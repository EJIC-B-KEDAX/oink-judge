#include "services/auth/ProtocolWithFastAPI.h"
#include "services/auth/HandleRequest.h"
#include <iostream>

namespace oink_judge::services::auth {

namespace {

[[maybe_unused]] bool registered = []() -> bool {
    socket::ProtocolFactory::instance().register_type(
        ProtocolWithFastAPI::REGISTERED_NAME,
        [](const std::string &params) -> std::unique_ptr<socket::Protocol> {
            return std::make_unique<ProtocolWithFastAPI>();
        }
    );
    return true;
}();

} // namespace

ProtocolWithFastAPI::ProtocolWithFastAPI() = default;

void ProtocolWithFastAPI::start(const std::string &start_message) {
    get_session()->receive_message();
}

void ProtocolWithFastAPI::receive_message(const std::string &message) {
    auto json_message = nlohmann::json::parse(message);
    get_session()->send_message(handle_client(json_message).dump());
    get_session()->receive_message();
}

void ProtocolWithFastAPI::close_session() {}

} // namespace oink_judge::services::auth
