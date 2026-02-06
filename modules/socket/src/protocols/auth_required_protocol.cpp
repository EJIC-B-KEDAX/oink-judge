#include "oink_judge/socket/protocols/auth_required_protocol.h"

#include <oink_judge/config/config.h>
#include <oink_judge/logger/logger.h>

namespace oink_judge::socket {

namespace {

[[maybe_unused]] const bool REGISTERED = []() -> bool {
    ProtocolFactory::instance().registerType(
        AuthRequiredProtocol::REGISTERED_NAME, [](const std::string& params) -> std::unique_ptr<Protocol> {
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

            return std::make_unique<AuthRequiredProtocol>(ProtocolFactory::instance().create(inner_type),
                                                          now_part[now_transition].get<std::string>());
        });
    return true;
}();

} // namespace

AuthRequiredProtocol::AuthRequiredProtocol(std::unique_ptr<Protocol> inner_protocol, std::string auth_token)
    : ProtocolDecorator(std::move(inner_protocol)), auth_token_(std::move(auth_token)) {}

auto AuthRequiredProtocol::start(std::string start_message) -> awaitable<void> {
    status_ = UNAUTHORIZED;
    saved_start_message_ = start_message;
    boost::asio::co_spawn(co_await boost::asio::this_coro::executor, getSession()->receiveMessage(), boost::asio::detached);
}

auto AuthRequiredProtocol::receiveMessage(std::string message) -> awaitable<void> {
    if (status_ == AUTHORIZED) {
        co_await ProtocolDecorator::receiveMessage(message);
    } else {
        if (message == auth_token_) {
            logger::logMessage("AuthRequiredProtocol", 2, "Success authorization", logger::DEBUG);
            status_ = AUTHORIZED;
            co_await ProtocolDecorator::start(saved_start_message_);
            saved_start_message_.clear();
        } else {
            logger::logMessage("AuthRequiredProtocol", 1, "Failed authorization attempt", logger::ERROR);
            closeSession();
        }
    }
}

void AuthRequiredProtocol::requestInternal(const std::string& message, const callback_t& callback) {
    if (status_ == AUTHORIZED) {
        ProtocolDecorator::requestInternal(message, callback);
    } else {
        logger::logMessage("AuthRequiredProtocol", 1, "Attempt to send request while not authorized", logger::ERROR);
        callCallback(callback,
                     std::make_error_code(std::errc::permission_denied)); // Not authorized, maybe it needs a better error code
    }
}

} // namespace oink_judge::socket
