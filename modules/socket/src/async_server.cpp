#include "oink_judge/socket/async_server.h"

#include "oink_judge/socket/boost_io_context.h"
#include "oink_judge/socket/byte_order.h"
#include "oink_judge/socket/session.hpp"

#include <nlohmann/json.hpp>
#include <oink_judge/logger/logger.h>
#include <utility>

namespace oink_judge::socket {

using json = nlohmann::json;

AsyncServer::AsyncServer(short port, std::shared_ptr<ConnectionHandler> handler)
    : acceptor_(BoostIOContext::instance(), tcp::endpoint(tcp::v4(), port)), handler_(std::move(handler)) {}

auto AsyncServer::startAccept() -> void { co_spawn(acceptor_.get_executor(), accept(), boost::asio::detached); }

awaitable<void> AsyncServer::accept() {
    try {
        tcp::socket socket = co_await acceptor_.async_accept(boost::asio::use_awaitable);
        co_spawn(co_await boost::asio::this_coro::executor, accept(), boost::asio::detached);

        uint64_t start_message_size_net = 0;

        try {
            co_await boost::asio::async_read(socket, boost::asio::buffer(&start_message_size_net, sizeof(start_message_size_net)),
                                             boost::asio::use_awaitable);

            size_t start_message_size = ntoh64(start_message_size_net);
            if (start_message_size == 0) {
                co_await handler_->newConnection(std::move(socket), "");
                co_return;
            }
            std::string start_message(start_message_size, '\0');

            co_await boost::asio::async_read(socket, boost::asio::buffer(start_message), boost::asio::use_awaitable);

            co_await handler_->newConnection(std::move(socket), std::move(start_message));
            co_return;
        } catch (const std::exception& e) {
            logger::logMessage("socket", 1, std::string("Error during connection setup: ") + e.what(), logger::ERROR);
            socket.close();
        }
    } catch (const std::exception& e) {
        logger::logMessage("socket", 1, std::string("Error accepting connection: ") + e.what(), logger::ERROR);
    }

    co_spawn(co_await boost::asio::this_coro::executor, accept(), boost::asio::detached);
}

} // namespace oink_judge::socket