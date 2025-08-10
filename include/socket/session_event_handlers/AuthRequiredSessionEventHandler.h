#pragma once

#include "socket/SessionEventHandler.hpp"

namespace oink_judge::socket {

class AuthRequiredSessionEventHandler : public SessionEventHandler {
public:
    AuthRequiredSessionEventHandler(std::unique_ptr<SessionEventHandler> inner_event_handler, std::string auth_token);

    void start(const std::string &start_message) override;
    void receive_message(const std::string &message) override;
    void close_session() override;

    void set_session(std::weak_ptr<Session> session) override;
    std::weak_ptr<Session> get_session() const override;

    constexpr static auto REGISTERED_NAME = "AuthRequired";

private:
    enum Status {
        UNAUTHORIZED,
        AUTHORIZED
    };

    Status _status = UNAUTHORIZED;
    std::unique_ptr<SessionEventHandler> _inner_event_handler;
    std::string _auth_token;
    std::string _saved_start_message;
};

} // namespace oink_judge::socket
