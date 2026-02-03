#pragma once

#include "oink_judge/socket/connection_handler.hpp"

#include <boost/asio.hpp>

namespace oink_judge::socket {

using tcp = boost::asio::ip::tcp;

class AsyncServer : public std::enable_shared_from_this<AsyncServer> {
  public:
    AsyncServer(short port, std::shared_ptr<ConnectionHandler> handler);
    virtual ~AsyncServer() = default;

    void start_accept();

  protected:
    virtual awaitable<void> accept();

  private:
    tcp::acceptor _acceptor;
    std::shared_ptr<ConnectionHandler> _handler;
};

} // namespace oink_judge::socket
