#pragma once

#include "socket/Protocol.hpp"

namespace oink_judge::socket {

class AuthorizingProtocol : public Protocol {
public:
    AuthorizingProtocol(std::unique_ptr<Protocol> inner_event_handler, std::string auth_token);

    void start(const std::string &start_message) override;
    void receive_message(const std::string &message) override;
    void close_session() override;

    void set_session(std::weak_ptr<Session> session) override;
    std::shared_ptr<Session> get_session() const override;

    constexpr static auto REGISTERED_NAME = "Authorizing";

    void request_internal(const std::string &message, const callback_t &callback) override;

private:
    std::unique_ptr<Protocol> _inner_protocol;
    std::string _auth_token;
};

} // namespace oink_judge::socket
