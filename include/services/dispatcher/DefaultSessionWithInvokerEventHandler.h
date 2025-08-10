#pragma once

#include "socket/SessionEventHandler.hpp"

namespace oink_judge::services::dispatcher {

class DefaultSessionWithInvokerEventHandler : public socket::SessionEventHandler {
public:
    DefaultSessionWithInvokerEventHandler();

    void start(const std::string &start_message) override;
    void receive_message(const std::string &message) override;
    void close_session() override;

    void set_session(std::weak_ptr<socket::Session> session) override;
    std::weak_ptr<socket::Session> get_session() const override;

    constexpr static auto REGISTERED_NAME = "DefaultSessionWithInvokerEventHandler";
    
private:
    std::string _invoker_id;
    std::weak_ptr<socket::Session> _session;
};

} // namespace oink_judge::services::dispatcher
