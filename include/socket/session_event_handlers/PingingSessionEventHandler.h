#pragma once

#include "socket/SessionEventHandler.hpp"
#include <boost/asio.hpp>

namespace oink_judge::socket {

class PingingSessionEventHandler : public SessionEventHandler {
public:
    PingingSessionEventHandler(std::unique_ptr<SessionEventHandler> inner_event_handler);

    void start(const std::string &start_message) override;
    void receive_message(const std::string &message) override;
    void close_session() override;

    void set_session(std::weak_ptr<Session> session) override;
    std::weak_ptr<Session> get_session() const override;

    constexpr static auto REGISTERED_NAME = "Pinging";

private:
    std::unique_ptr<SessionEventHandler> _inner_event_handler;
    float _ping_interval_seconds;
    float _pong_timeout_seconds;
    boost::asio::steady_timer _ping_timer;
    boost::asio::steady_timer _pong_timer;

    void ping_loop();
    void wait_for_pong();
};

} // namespace oink_judge::socket
