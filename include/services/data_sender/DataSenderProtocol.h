#pragma once
#include "socket/protocols/ProtocolBase.h"

#include <string>

namespace oink_judge::services::data_sender {

using boost::asio::awaitable;
using Session = socket::Session;

class DataSenderProtocol : public socket::ProtocolBase {
  public:
    DataSenderProtocol();

    awaitable<void> start(std::string start_message) override;
    awaitable<void> receive_message(std::string message) override;
    void close_session() override;

    constexpr static auto REGISTERED_NAME = "DataSenderProtocol";
};

} // namespace oink_judge::services::data_sender
