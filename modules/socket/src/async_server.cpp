#include "oink_judge/socket/async_server.h"

#include "oink_judge/socket/boost_io_context.h"
#include "oink_judge/socket/byte_order.h"
#include "oink_judge/socket/session.hpp"

#include <iostream>
#include <nlohmann/json.hpp>
#include <utility>

namespace oink_judge::socket {

using json = nlohmann::json;

AsyncServer::AsyncServer(short port, std::shared_ptr<ConnectionHandler> handler)
    : _acceptor(BoostIOContext::instance(), tcp::endpoint(tcp::v4(), port)), _handler(std::move(handler)) {}

void AsyncServer::start_accept() { co_spawn(_acceptor.get_executor(), accept(), boost::asio::detached); }

awaitable<void> AsyncServer::accept() {
    try {
        tcp::socket socket = co_await _acceptor.async_accept(boost::asio::use_awaitable);
        co_spawn(co_await boost::asio::this_coro::executor, accept(), boost::asio::detached);

        std::cout << "Accepted new connection" << std::endl;
        uint64_t start_message_size_net = 0;

        try {
            co_await boost::asio::async_read(socket, boost::asio::buffer(&start_message_size_net, sizeof(start_message_size_net)),
                                             boost::asio::use_awaitable);

            size_t start_message_size = ntoh64(start_message_size_net);
            if (start_message_size == 0) {
                co_await _handler->new_connection(socket, "");
                co_return;
            }
            std::string start_message(start_message_size, '\0');

            co_await boost::asio::async_read(socket, boost::asio::buffer(start_message), boost::asio::use_awaitable);

            std::cout << "Read start message: " << start_message << std::endl;

            co_await _handler->new_connection(socket, start_message);

            co_return;

        } catch (const std::exception& e) {
            std::cerr << "Error during connection setup: " << e.what() << std::endl;
            socket.close();
        }
    } catch (const std::exception& e) {
        std::cerr << "Error accepting connection: " << e.what() << std::endl;
    }

    co_spawn(co_await boost::asio::this_coro::executor, accept(), boost::asio::detached);
}

} // namespace oink_judge::socket