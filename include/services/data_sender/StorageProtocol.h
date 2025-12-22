#pragma once

#include <string>
#include "socket/protocols/ProtocolWithRequests.h"

namespace oink_judge::services::data_sender {

using Session = socket::Session;

class StorageProtocol : public socket::ProtocolWithRequests {
public:
    StorageProtocol();

    void start(const std::string &start_message) override;
    void receive_message(const std::string &message) override;
    void close_session() override;

    constexpr static auto REGISTERED_NAME = "StorageProtocol";
    
private:
    enum Status {
        WAIT_HEADER,
        WAIT_DATA
    };
    Status _status;
    uint64_t _now_response_id;
    std::string _content_type;
    std::string _content_id;
};

} // namespace oink_judge::services::data_sender
