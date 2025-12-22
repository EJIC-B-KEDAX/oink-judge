#pragma once

#include "socket/protocols/ProtocolDecorator.h"

namespace oink_judge::socket {

class PongingProtocol : public ProtocolDecorator {
public:
    PongingProtocol(std::unique_ptr<Protocol> inner_protocol);

    void receive_message(const std::string &message) override;

    constexpr static auto REGISTERED_NAME = "Ponging";
};

} // namespace oink_judge::socket
