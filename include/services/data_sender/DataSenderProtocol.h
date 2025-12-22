#pragma once

#include <string>
#include "socket/protocols/ProtocolBase.h"

namespace oink_judge::services::data_sender {

using Session = socket::Session;

class DataSenderProtocol : public socket::ProtocolBase {
public:
    DataSenderProtocol();

    void start(const std::string &start_message) override;
    void receive_message(const std::string &message) override;
    void close_session() override;;

    constexpr static auto REGISTERED_NAME = "DataSenderProtocol";

private:
    enum Status {
        WAIT_REQUEST,
        WAIT_DATA
    };
    Status _status;
    std::string _content_type;
    std::string _content_id;
};

} // namespace oink_judge::services::data_sender
