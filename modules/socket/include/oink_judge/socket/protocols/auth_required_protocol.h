#pragma once
#include "oink_judge/socket/protocols/protocol_decorator.h"

#include <sys/types.h>

namespace oink_judge::socket {

class AuthRequiredProtocol : public ProtocolDecorator {
  public:
    AuthRequiredProtocol(std::unique_ptr<Protocol> inner_protocol, std::string auth_token);

    auto start(std::string start_message) -> awaitable<void> override;
    auto receiveMessage(std::string message) -> awaitable<void> override;

    constexpr static auto REGISTERED_NAME = "AuthRequired";

    auto requestInternal(const std::string& message, const callback_t& callback) -> void override;

  private:
    enum Status : uint8_t { UNAUTHORIZED, AUTHORIZED };

    Status status_ = UNAUTHORIZED;
    std::string auth_token_;
    std::string saved_start_message_;
};

} // namespace oink_judge::socket
