#pragma once

#include "socket/protocols/ProtocolBase.h"

namespace oink_judge::services::dispatcher {

class ProtocolWithInvoker : public socket::ProtocolBase {
public:
    ProtocolWithInvoker();

    void start(const std::string &start_message) override;
    void receive_message(const std::string &message) override;
    void close_session() override;

    constexpr static auto REGISTERED_NAME = "ProtocolWithInvoker";
    
private:
    std::string _invoker_id;
};

} // namespace oink_judge::services::dispatcher
