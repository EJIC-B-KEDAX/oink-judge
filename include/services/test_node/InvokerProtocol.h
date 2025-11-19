#pragma once
#include "socket/Protocol.hpp"

namespace oink_judge::services::test_node {

class InvokerProtocol : public socket::Protocol {
public:
    InvokerProtocol();

    void start(const std::string &start_message) override;
    void receive_message(const std::string &message) override;
    void close_session() override;

    void set_session(std::weak_ptr<socket::Session> session) override;
    std::shared_ptr<socket::Session> get_session() const override;

    constexpr static auto REGISTERED_NAME = "InvokerProtocol";

    void request_internal(const std::string &message, const callback_t &callback) override;

private:
    std::weak_ptr<socket::Session> _session;
};

} // namespace oink_judge::services::test_node
