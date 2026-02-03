#pragma once
#include "socket/protocols/ProtocolWithRequests.h"

#include <string>

namespace oink_judge::services::data_sender {

using boost::asio::awaitable;
using Session = socket::Session;

class StorageProtocol : public socket::ProtocolWithRequests {
  public:
    StorageProtocol();

    awaitable<void> start(std::string start_message) override;
    awaitable<void> receive_message(std::string message) override;
    void close_session() override;

    constexpr static auto REGISTERED_NAME = "StorageProtocol";
};

} // namespace oink_judge::services::data_sender
