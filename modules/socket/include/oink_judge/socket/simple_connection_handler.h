#pragma once
#include "oink_judge/socket/connection_handler.hpp"

namespace oink_judge::socket {

using tcp = boost::asio::ip::tcp;

class SimpleConnectionHandler : public ConnectionHandler {
  public:
    SimpleConnectionHandler();

    auto newConnection(tcp::socket socket, std::string start_message) -> awaitable<void> override;

    constexpr static auto REGISTERED_NAME = "SimpleConnectionHandler";
};

} // namespace oink_judge::socket
