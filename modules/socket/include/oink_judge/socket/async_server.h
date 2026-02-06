#pragma once
#include "oink_judge/socket/connection_handler.hpp"

#include <boost/asio.hpp>

namespace oink_judge::socket {

using tcp = boost::asio::ip::tcp;

class AsyncServer : public std::enable_shared_from_this<AsyncServer> {
  public:
    AsyncServer(const AsyncServer&) = delete;
    auto operator=(const AsyncServer&) -> AsyncServer& = delete;
    AsyncServer(AsyncServer&&) = delete;
    auto operator=(AsyncServer&&) -> AsyncServer& = delete;
    virtual ~AsyncServer() = default;

    AsyncServer(short port, std::shared_ptr<ConnectionHandler> handler);

    void startAccept();

  protected:
    virtual auto accept() -> awaitable<void>;

  private:
    tcp::acceptor acceptor_;
    std::shared_ptr<ConnectionHandler> handler_;
};

} // namespace oink_judge::socket
