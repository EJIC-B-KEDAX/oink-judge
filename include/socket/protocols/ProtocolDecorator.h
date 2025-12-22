#pragma once
#include "socket/protocols/ProtocolWithRequests.h"

namespace oink_judge::socket {

class ProtocolDecorator : public Protocol {
public:
    ProtocolDecorator(std::unique_ptr<Protocol> inner_protocol);

    void start(const std::string &start_message) override;
    void send_message(const std::string &message) override;
    void receive_message(const std::string &message) override;
    void close_session() override;

    void set_session(std::weak_ptr<Session> session) override;
    std::shared_ptr<Session> get_session() const override;

    void request_internal(const std::string &message, const callback_t &callback) override;

protected:
    std::unique_ptr<Protocol> &access_inner_protocol();
    
private:
    std::unique_ptr<Protocol> _inner_protocol;
};

} // namespace oink_judge::socket
