#pragma once
#include <oink_judge/socket/protocols/protocol_base.h>

namespace oink_judge::dispatcher {

using boost::asio::awaitable;

class ProtocolWithInvoker : public socket::ProtocolBase {
  public:
    ProtocolWithInvoker();

    auto start(std::string start_message) -> awaitable<void> override;
    auto receiveMessage(std::string message) -> awaitable<void> override;
    auto closeSession() -> void override;

    constexpr static auto REGISTERED_NAME = "ProtocolWithInvoker";

  private:
    std::string invoker_id_;
};

} // namespace oink_judge::dispatcher
