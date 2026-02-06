#pragma once
#include "oink_judge/socket/protocols/protocol_decorator.h"

#include <boost/asio.hpp>

namespace oink_judge::socket {

class PingingProtocol : public ProtocolDecorator {
  public:
    PingingProtocol(std::unique_ptr<Protocol> inner_protocol);

    auto start(std::string start_message) -> awaitable<void> override;
    auto receiveMessage(std::string message) -> awaitable<void> override;
    auto closeSession() -> void override;

    constexpr static auto REGISTERED_NAME = "Pinging";

  private:
    std::chrono::duration<double> ping_interval_seconds_;
    std::chrono::duration<double> pong_timeout_seconds_;
    boost::asio::steady_timer ping_timer_;
    boost::asio::steady_timer pong_timer_;

    auto pingLoop() -> void;
    auto waitForPong() -> void;
};

} // namespace oink_judge::socket
