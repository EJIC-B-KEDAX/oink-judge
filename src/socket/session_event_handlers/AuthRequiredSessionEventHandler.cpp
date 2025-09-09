#include "socket/session_event_handlers/AuthRequiredSessionEventHandler.h"
#include "config/Config.h"
#include <iostream>

namespace oink_judge::socket {

namespace {

[[maybe_unused]] bool registered = []() {
    BasicSessionEventHandlerFactory::instance().register_type(AuthRequiredSessionEventHandler::REGISTERED_NAME,
        [](const std::string &params) -> std::unique_ptr<SessionEventHandler> {
        auto json_params = nlohmann::json::parse(params);
        if (!json_params.contains("inner_type") || !json_params.contains("auth_token_path")) {
            throw std::runtime_error("AuthRequiredSessionEventHandler requires 'inner_type' and 'auth_token_path' parameters");
        }

        if (!json_params["inner_type"].is_string() || !json_params["auth_token_path"].is_string()) {
            throw std::runtime_error("AuthRequiredSessionEventHandler parameters must be strings");
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

        return std::make_unique<AuthRequiredSessionEventHandler>(BasicSessionEventHandlerFactory::instance().create(inner_type), now_part[now_transition].get<std::string>());
    });
    return true;
}();

} // namespace

AuthRequiredSessionEventHandler::AuthRequiredSessionEventHandler(std::unique_ptr<SessionEventHandler> inner_event_handler, std::string auth_token)
    : _inner_event_handler(std::move(inner_event_handler)), _auth_token(std::move(auth_token)) {}

void AuthRequiredSessionEventHandler::start(const std::string &start_message) {
    std::cout << "Client connected, waiting for authorization..." << std::endl;
    _status = UNAUTHORIZED;
    _saved_start_message = start_message;
    get_session().lock()->receive_message();
}

void AuthRequiredSessionEventHandler::receive_message(const std::string &message) {
    if (_status == AUTHORIZED) {
        _inner_event_handler->receive_message(message);
    } else {
        std::cout << "Received authorization token, checking..." << std::endl;
        std::cout << "Expected: " << _auth_token << ", got: " << message << std::endl;
        if (message == _auth_token) {
            _status = AUTHORIZED;
            _inner_event_handler->start(_saved_start_message);
            _saved_start_message.clear();
        } else {
            close_session();
        }
    }
}

void AuthRequiredSessionEventHandler::close_session() {
    _inner_event_handler->close_session();
}

void AuthRequiredSessionEventHandler::set_session(std::weak_ptr<Session> session) {
    std::cout << "Setting session in AuthRequiredSessionEventHandler" << std::endl;
    _inner_event_handler->set_session(session);
}

std::weak_ptr<Session> AuthRequiredSessionEventHandler::get_session() const {
    return _inner_event_handler->get_session();
}

} // namespace oink_judge::socket
