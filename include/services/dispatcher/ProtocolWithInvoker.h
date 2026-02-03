#pragma once
#include "socket/protocols/ProtocolBase.h"

namespace oink_judge::services::dispatcher {

using boost::asio::awaitable;

class ProtocolWithInvoker : public socket::ProtocolBase {
  public:
    ProtocolWithInvoker();

    auto start(std::string start_message) -> awaitable<void> override;
    auto receive_message(std::string message) -> awaitable<void> override;
    auto close_session() -> void override;

    constexpr static auto REGISTERED_NAME = "ProtocolWithInvoker";

  private:
    std::string invoker_id_;
};

} // namespace oink_judge::services::dispatcher
