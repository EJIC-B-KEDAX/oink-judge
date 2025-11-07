#include "services/auth/SessionWithFastAPIEventHandler.h"
#include "services/auth/HandleRequest.h"
#include <iostream>

namespace oink_judge::services::auth {

namespace {

[[maybe_unused]] bool registered = []() -> bool {
    socket::BasicSessionEventHandlerFactory::instance().register_type(
        SessionWithFastAPIEventHandler::REGISTERED_NAME,
        [](const std::string &params) -> std::unique_ptr<socket::SessionEventHandler> {
            return std::make_unique<SessionWithFastAPIEventHandler>();
        }
    );
    return true;
}();

} // namespace

SessionWithFastAPIEventHandler::SessionWithFastAPIEventHandler() = default;

void SessionWithFastAPIEventHandler::start(const std::string &start_message) {
    get_session().lock()->receive_message();
}

void SessionWithFastAPIEventHandler::receive_message(const std::string &message) {
    auto json_message = nlohmann::json::parse(message);
    std::cout << "Parsed JSON message: " << json_message.dump() << std::endl;
    get_session().lock()->send_message(handle_client(json_message).dump());
    get_session().lock()->receive_message();
}

void SessionWithFastAPIEventHandler::close_session() {
    std::cout << "Closing session." << std::endl;
}

void SessionWithFastAPIEventHandler::set_session(std::weak_ptr<socket::Session> session) {
    _session = session;
}

std::weak_ptr<socket::Session> SessionWithFastAPIEventHandler::get_session() const {
    return _session;
}

} // namespace oink_judge::services::auth
