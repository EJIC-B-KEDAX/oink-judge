#pragma once

#include <boost/asio.hpp>
#include <oink_judge>

namespace oink_judge::socket {

using tcp = boost::asio::ip::tcp;
using boost::asio::awaitable;

class ConnectionHandler {
  public:
    virtual ~ConnectionHandler() = default;

    virtual awaitable<void> new_connection(tcp::socket& socket, const std::string& start_message) = 0;
};

using ConnectionHandlerFactory = factory::ParameterizedTypeFactory<std::shared_ptr<ConnectionHandler>>;

} // namespace oink_judge::socket
