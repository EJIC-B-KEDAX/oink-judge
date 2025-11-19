#pragma once

#include "socket/SessionEventHandler.hpp"

namespace oink_judge::socket {

class PongingSessionEventHandler : public SessionEventHandler {
public:
    PongingSessionEventHandler(std::unique_ptr<SessionEventHandler> inner_event_handler);

    void start(const std::string &start_message) override;
    void receive_message(const std::string &message) override;
    void close_session() override;

    void set_session(std::weak_ptr<Session> session) override;
    std::shared_ptr<Session> get_session() const override;

    constexpr static auto REGISTERED_NAME = "Ponging";

    void request_internal(const std::string &message, const callback_t &callback) override;

private:
    std::unique_ptr<SessionEventHandler> _inner_event_handler;
};

} // namespace oink_judge::socket
