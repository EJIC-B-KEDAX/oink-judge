#pragma once

#include <string>
#include "socket/Protocol.hpp"

namespace oink_judge::services::data_sender {

using Session = socket::Session;

class DataSenderProtocol : public socket::Protocol {
public:
    DataSenderProtocol();

    void start(const std::string &start_message) override;
    void receive_message(const std::string &message) override;
    void close_session() override;

    void set_session(std::weak_ptr<socket::Session> session) override;
    std::shared_ptr<socket::Session> get_session() const override;

    constexpr static auto REGISTERED_NAME = "DataSenderProtocol";

    void request_internal(const std::string &message, const callback_t &callback) override;
    
private:
    std::weak_ptr<socket::Session> _session;
};

} // namespace oink_judge::services::data_sender
