#pragma once
#include <oink_judge/socket/protocols/protocol_base.h>

namespace oink_judge::auth_service {

using boost::asio::awaitable;

class ProtocolWithFastAPI : public socket::ProtocolBase {
  public:
    ProtocolWithFastAPI();

    auto start(std::string start_message) -> awaitable<void> override;
    auto receiveMessage(std::string message) -> awaitable<void> override;
    auto closeSession() -> void override;

    constexpr static auto REGISTERED_NAME = "ProtocolWithFastAPI";
};

} // namespace oink_judge::auth_service
