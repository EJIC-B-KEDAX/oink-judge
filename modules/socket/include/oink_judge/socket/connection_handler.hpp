#pragma once
#include <boost/asio.hpp>
#include <oink_judge/factory/parameterized_type_factory.hpp>

namespace oink_judge::socket {

using tcp = boost::asio::ip::tcp;
using boost::asio::awaitable;

class ConnectionHandler {
  public:
    ConnectionHandler(const ConnectionHandler&) = delete;
    auto operator=(const ConnectionHandler&) -> ConnectionHandler& = delete;
    ConnectionHandler(ConnectionHandler&&) = delete;
    auto operator=(ConnectionHandler&&) -> ConnectionHandler& = delete;
    virtual ~ConnectionHandler() = default;

    virtual auto newConnection(tcp::socket socket, std::string start_message) -> awaitable<void> = 0;

  protected:
    ConnectionHandler() = default;
};

using ConnectionHandlerFactory = factory::ParameterizedTypeFactory<std::shared_ptr<ConnectionHandler>>;

} // namespace oink_judge::socket
