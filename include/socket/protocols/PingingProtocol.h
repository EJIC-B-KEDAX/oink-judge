#pragma once

#include "socket/protocols/ProtocolDecorator.h"
#include <boost/asio.hpp>

namespace oink_judge::socket {

class PingingProtocol : public ProtocolDecorator {
public:
    PingingProtocol(std::unique_ptr<Protocol> inner_protocol);

    void start(const std::string &start_message) override;
    void receive_message(const std::string &message) override;
    void close_session() override;

    constexpr static auto REGISTERED_NAME = "Pinging";

private:
    float _ping_interval_seconds;
    float _pong_timeout_seconds;
    boost::asio::steady_timer _ping_timer;
    boost::asio::steady_timer _pong_timer;

    void ping_loop();
    void wait_for_pong();
};

} // namespace oink_judge::socket
