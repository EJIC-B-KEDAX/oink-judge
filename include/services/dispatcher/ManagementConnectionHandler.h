#pragma once
#include "socket/ConnectionHandler.hpp"

namespace oink_judge::services::dispatcher {

using tcp = boost::asio::ip::tcp;

class ManagementConnectionHandler : public socket::ConnectionHandler {
public:
    void new_connection(tcp::socket &socket, const std::string &start_message) override;

};

} // namespace oink_judge::services::dispatcher
