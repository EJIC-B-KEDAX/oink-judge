#pragma once

#include "socket/protocols/ProtocolBase.h"

namespace oink_judge::services::auth {

class ProtocolWithFastAPI : public socket::ProtocolBase {
public:
    ProtocolWithFastAPI();

    void start(const std::string &start_message) override;
    void receive_message(const std::string &message) override;
    void close_session() override;

    constexpr static auto REGISTERED_NAME = "ProtocolWithFastAPI";
};

} // namespace oink_judge::services::auth
