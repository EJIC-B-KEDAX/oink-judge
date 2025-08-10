#include "socket/AsyncServer.h"
#include <nlohmann/json.hpp>
#include <utility>
#include "socket/Session.hpp"
#include "socket/BoostIOContext.h"
#include "socket/byte_order.h"

namespace oink_judge::socket {

using json = nlohmann::json;

AsyncServer::AsyncServer(short port, std::shared_ptr<ConnectionHandler> handler) :
    _acceptor(BoostIOContext::instance(), tcp::endpoint(tcp::v4(), port)), _handler(std::move(handler)) {}

void AsyncServer::start_accept() {
    accept();
}

void AsyncServer::accept() {
    _acceptor.async_accept([this](boost::system::error_code ec, tcp::socket socket) -> void {
        if (!ec) {
            size_t start_message_size_net;
            auto socket_ptr = std::make_shared<tcp::socket>(std::move(socket));
            boost::asio::async_read(*socket_ptr, boost::asio::buffer(&start_message_size_net, sizeof start_message_size_net),
                [this, start_message_size_net, socket_ptr](boost::system::error_code ec, std::size_t /*length*/) -> void {
                if (!ec) {
                    size_t start_message_size = ntoh64(start_message_size_net);
                    if (start_message_size == 0) {
                        _handler->new_connection(*socket_ptr, "");
                        return;
                    }
                    std::string start_message(start_message_size, '\0');

                    boost::asio::async_read(*socket_ptr, boost::asio::buffer(start_message),
                        [this, start_message, socket_ptr](boost::system::error_code ec, std::size_t /*length*/) -> void {
                        if (!ec) {
                            _handler->new_connection(*socket_ptr, start_message);
                        } else {
                            socket_ptr->close();
                        }
                    });
                } else {
                    socket_ptr->close();
                }
            });
        }
        accept();
    });
}

} // namespace oink_judge::socket