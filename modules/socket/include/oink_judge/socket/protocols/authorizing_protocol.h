#pragma once

#include "socket/protocols/ProtocolDecorator.h"

namespace oink_judge::socket {

class AuthorizingProtocol : public ProtocolDecorator {
public:
    AuthorizingProtocol(std::unique_ptr<Protocol> inner_protocols, std::string auth_token);

    awaitable<void> start(const std::string &start_message) override;

    void request_internal(const std::string &message, const callback_t &callback) override;

    constexpr static auto REGISTERED_NAME = "Authorizing";

private:
    std::string _auth_token;
    bool _authorized;
    std::vector<std::pair<std::string, callback_t>> _pending_requests;
};

} // namespace oink_judge::socket
