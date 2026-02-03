#pragma once
#include "socket/protocols/ProtocolBase.h"

namespace oink_judge::services::test_node {

using boost::asio::awaitable;

class InvokerProtocol : public socket::ProtocolBase {
public:
    InvokerProtocol();

    awaitable<void> start(const std::string &start_message) override;
    awaitable<void> receive_message(const std::string &message) override;
    void close_session() override;

    constexpr static auto REGISTERED_NAME = "InvokerProtocol";
};

} // namespace oink_judge::services::test_node
