#include "oink_judge/socket/protocols/authorizing_protocol.h"

#include <oink_judge/config/config.h>

namespace oink_judge::socket {

namespace {

[[maybe_unused]] const bool REGISTERED = []() -> bool {
    ProtocolFactory::instance().registerType(
        AuthorizingProtocol::REGISTERED_NAME, [](const std::string& params) -> std::unique_ptr<Protocol> {
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

            std::string now_transition;
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

            return std::make_unique<AuthorizingProtocol>(ProtocolFactory::instance().create(inner_type),
                                                         now_part[now_transition].get<std::string>());
        });
    return true;
}();

} // namespace

AuthorizingProtocol::AuthorizingProtocol(std::unique_ptr<Protocol> inner_protocol, std::string auth_token)
    : ProtocolDecorator(std::move(inner_protocol)), auth_token_(std::move(auth_token)) {}

auto AuthorizingProtocol::start(std::string start_message) -> awaitable<void> {
    co_await sendMessage(auth_token_);
    co_await ProtocolDecorator::start(start_message);
}

void AuthorizingProtocol::requestInternal(const std::string& message, const callback_t& callback) {
    ProtocolDecorator::requestInternal(message, callback);
}

} // namespace oink_judge::socket
