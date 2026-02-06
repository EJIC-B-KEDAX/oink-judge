#pragma once
#include "oink_judge/socket/protocols/protocol_decorator.h"

namespace oink_judge::socket {

class AuthorizingProtocol : public ProtocolDecorator {
  public:
    AuthorizingProtocol(std::unique_ptr<Protocol> inner_protocols, std::string auth_token);

    auto start(std::string start_message) -> awaitable<void> override;

    auto requestInternal(const std::string& message, const callback_t& callback) -> void override;

    constexpr static auto REGISTERED_NAME = "Authorizing";

  private:
    std::string auth_token_;
};

} // namespace oink_judge::socket
