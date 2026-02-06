#pragma once
#include "oink_judge/socket/protocols/protocol_decorator.h"

namespace oink_judge::socket {

class PongingProtocol : public ProtocolDecorator {
  public:
    PongingProtocol(std::unique_ptr<Protocol> inner_protocol);

    auto receiveMessage(std::string message) -> awaitable<void> override;

    constexpr static auto REGISTERED_NAME = "Ponging";
};

} // namespace oink_judge::socket
