#include "socket/session_event_handlers/AuthorizingSessionEventHandler.h"
#include "config/Config.h"

namespace oink_judge::socket {

namespace {

[[maybe_unused]] bool registered = []() {
    BasicSessionEventHandlerFactory::instance().register_type(AuthorizingSessionEventHandler::REGISTERED_NAME,
        [](const std::string &params) -> std::unique_ptr<SessionEventHandler> {
        auto json_params = nlohmann::json::parse(params);
        if (!json_params.contains("inner_type") || !json_params.contains("auth_token_path")) {
            throw std::runtime_error("AuthorizingSessionEventHandler requires 'inner_type' and 'auth_token_path' parameters");
        }

        if (!json_params["inner_type"].is_string() || !json_params["auth_token_path"].is_string()) {
            throw std::runtime_error("AuthorizingSessionEventHandler parameters must be strings");
        }

        auto inner_type = json_params["inner_type"].get<std::string>();
        auto auth_token_path = json_params["auth_token_path"].get<std::string>();

        nlohmann::json now_part = config::Config::credentials();

        std::string now_transition = "";
        for (char c : auth_token_path) {
            if (c == '.') {
                if (!now_part.contains(now_transition)) {
                    throw std::runtime_error("Auth token path '" + auth_token_path + "' does not exist in credentials");
                }
                now_part = now_part[now_transition];
                now_transition.clear();
            } else {
                now_transition += c;
            }
        }

        if (!now_part.contains(now_transition) || !now_part[now_transition].is_string()) {
            throw std::runtime_error("Auth token path '" + auth_token_path + "' does not exist in credentials");
        }

        return std::make_unique<AuthorizingSessionEventHandler>(BasicSessionEventHandlerFactory::instance().create(inner_type), now_part[now_transition].get<std::string>());
    });
    return true;
}();

} // namespace

AuthorizingSessionEventHandler::AuthorizingSessionEventHandler(std::unique_ptr<SessionEventHandler> inner_event_handler, std::string auth_token)
    : _inner_event_handler(std::move(inner_event_handler)), _auth_token(std::move(auth_token)) {}

void AuthorizingSessionEventHandler::start(const std::string &start_message) {
    get_session()->send_message(_auth_token);
    _inner_event_handler->start(start_message);
}

void AuthorizingSessionEventHandler::receive_message(const std::string &message) {
    _inner_event_handler->receive_message(message);
}

void AuthorizingSessionEventHandler::close_session() {
    _inner_event_handler->close_session();
}

void AuthorizingSessionEventHandler::set_session(std::weak_ptr<Session> session) {
    _inner_event_handler->set_session(session);
}

std::shared_ptr<Session> AuthorizingSessionEventHandler::get_session() const {
    return _inner_event_handler->get_session();
}

void AuthorizingSessionEventHandler::request_internal(const std::string &message, const callback_t &callback) {
    _inner_event_handler->request_internal(message, callback);
}

} // namespace oink_judge::socket
