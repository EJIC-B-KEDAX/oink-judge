#pragma once

#include "socket/ConnectionHandler.hpp"

namespace oink_judge::socket {

using tcp = boost::asio::ip::tcp;

class SimpleConnectionHandler : public ConnectionHandler {
public:
    SimpleConnectionHandler();

    awaitable<void> new_connection(tcp::socket &socket, const std::string &start_message) override;

    constexpr static auto REGISTERED_NAME = "SimpleConnectionHandler";
};

} // namespace oink_judge::socket
