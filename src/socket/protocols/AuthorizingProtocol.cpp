#include "socket/protocols/AuthorizingProtocol.h"
#include "config/Config.h"

namespace oink_judge::socket {

namespace {

[[maybe_unused]] bool registered = []() {
    ProtocolFactory::instance().register_type(AuthorizingProtocol::REGISTERED_NAME,
        [](const std::string &params) -> std::unique_ptr<Protocol> {
        auto json_params = nlohmann::json::parse(params);
        if (!json_params.contains("inner_type") || !json_params.contains("auth_token_path")) {
            throw std::runtime_error("AuthorizingProtocol requires 'inner_type' and 'auth_token_path' parameters");
        }

        if (!json_params["inner_type"].is_string() || !json_params["auth_token_path"].is_string()) {
            throw std::runtime_error("AuthorizingProtocol parameters must be strings");
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

        return std::make_unique<AuthorizingProtocol>(ProtocolFactory::instance().create(inner_type), now_part[now_transition].get<std::string>());
    });
    return true;
}();

} // namespace

AuthorizingProtocol::AuthorizingProtocol(std::unique_ptr<Protocol> inner_protocol, std::string auth_token)
    : _inner_protocol(std::move(inner_protocol)), _auth_token(std::move(auth_token)) {}

void AuthorizingProtocol::start(const std::string &start_message) {
    get_session()->send_message(_auth_token);
    _inner_protocol->start(start_message);
}

void AuthorizingProtocol::receive_message(const std::string &message) {
    _inner_protocol->receive_message(message);
}

void AuthorizingProtocol::close_session() {
    _inner_protocol->close_session();
}

void AuthorizingProtocol::set_session(std::weak_ptr<Session> session) {
    _inner_protocol->set_session(session);
}

std::shared_ptr<Session> AuthorizingProtocol::get_session() const {
    return _inner_protocol->get_session();
}

void AuthorizingProtocol::request_internal(const std::string &message, const callback_t &callback) {
    _inner_protocol->request_internal(message, callback);
}

} // namespace oink_judge::socket
