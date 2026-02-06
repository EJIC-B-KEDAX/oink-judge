#pragma once
#include <oink_judge/socket/protocols/protocol_base.h>

namespace oink_judge::test_node {

using boost::asio::awaitable;

class InvokerProtocol : public socket::ProtocolBase {
  public:
    InvokerProtocol();

    auto start(std::string start_message) -> awaitable<void> override;
    auto receiveMessage(std::string message) -> awaitable<void> override;
    auto closeSession() -> void override;

    constexpr static auto REGISTERED_NAME = "InvokerProtocol";
};

} // namespace oink_judge::test_node
