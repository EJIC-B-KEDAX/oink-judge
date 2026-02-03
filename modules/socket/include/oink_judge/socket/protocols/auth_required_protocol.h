#pragma once

#include "socket/protocols/ProtocolDecorator.h"

namespace oink_judge::socket {

class AuthRequiredProtocol : public ProtocolDecorator {
public:
    AuthRequiredProtocol(std::unique_ptr<Protocol> inner_protocol, std::string auth_token);

    awaitable<void> start(const std::string &start_message) override;
    awaitable<void> receive_message(const std::string &message) override;

    constexpr static auto REGISTERED_NAME = "AuthRequired";

    void request_internal(const std::string &message, const callback_t &callback) override;

private:
    enum Status {
        UNAUTHORIZED,
        AUTHORIZED
    };

    Status _status = UNAUTHORIZED;
    std::string _auth_token;
    std::string _saved_start_message;
};

} // namespace oink_judge::socket
