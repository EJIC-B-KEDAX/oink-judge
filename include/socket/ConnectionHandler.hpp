#pragma once

#include <boost/asio.hpp>
#include "ParameterizedTypeFactory.h"

namespace oink_judge::socket {

using tcp = boost::asio::ip::tcp;

class ConnectionHandler {
public:
    virtual ~ConnectionHandler() = default;

    virtual void new_connection(tcp::socket &socket, const std::string &start_message) = 0;
};

using BasicConnectionHandlerFactory = ParameterizedTypeFactory<std::shared_ptr<ConnectionHandler>>;

} // namespace oink_judge::socket
