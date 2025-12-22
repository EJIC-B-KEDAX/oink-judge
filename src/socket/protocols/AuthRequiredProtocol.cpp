#include "socket/protocols/AuthRequiredProtocol.h"
#include "config/Config.h"
#include <iostream>

namespace oink_judge::socket {

namespace {

[[maybe_unused]] bool registered = []() {
    ProtocolFactory::instance().register_type(AuthRequiredProtocol::REGISTERED_NAME,
        [](const std::string &params) -> std::unique_ptr<Protocol> {
        auto json_params = nlohmann::json::parse(params);
        if (!json_params.contains("inner_type") || !json_params.contains("auth_token_path")) {
            throw std::runtime_error("AuthRequiredProtocol requires 'inner_type' and 'auth_token_path' parameters");
        }

        if (!json_params["inner_type"].is_string() || !json_params["auth_token_path"].is_string()) {
            throw std::runtime_error("AuthRequiredProtocol parameters must be strings");
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

        return std::make_unique<AuthRequiredProtocol>(ProtocolFactory::instance().create(inner_type), now_part[now_transition].get<std::string>());
    });
    return true;
}();

} // namespace

AuthRequiredProtocol::AuthRequiredProtocol(std::unique_ptr<Protocol> inner_protocol, std::string auth_token)
    : ProtocolDecorator(std::move(inner_protocol)), _auth_token(std::move(auth_token)) {}

void AuthRequiredProtocol::start(const std::string &start_message) {
    std::cout << "Client connected, waiting for authorization..." << std::endl;
    _status = UNAUTHORIZED;
    _saved_start_message = start_message;
    get_session()->receive_message();
}

void AuthRequiredProtocol::receive_message(const std::string &message) {
    if (_status == AUTHORIZED) {
        ProtocolDecorator::receive_message(message);
    } else {
        std::cout << "Received authorization token, checking..." << std::endl;
        std::cout << "Expected: " << _auth_token << ", got: " << message << std::endl;
        if (message == _auth_token) {
            _status = AUTHORIZED;
            ProtocolDecorator::start(_saved_start_message);
            _saved_start_message.clear();
        } else {
            close_session();
        }
    }
}

void AuthRequiredProtocol::request_internal(const std::string &message, const callback_t &callback) {
    if (_status == AUTHORIZED) {
        ProtocolDecorator::request_internal(message, callback);
    } else {
        call_callback(callback, std::make_error_code(std::errc::permission_denied)); // Not authorized, maybe it needs a better error code
    }
}

} // namespace oink_judge::socket
