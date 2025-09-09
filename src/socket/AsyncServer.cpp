#include "socket/AsyncServer.h"
#include <nlohmann/json.hpp>
#include <utility>
#include "socket/Session.hpp"
#include "socket/BoostIOContext.h"
#include "socket/byte_order.h"
#include <iostream>

namespace oink_judge::socket {

using json = nlohmann::json;

AsyncServer::AsyncServer(short port, std::shared_ptr<ConnectionHandler> handler) :
    _acceptor(BoostIOContext::instance(), tcp::endpoint(tcp::v4(), port)), _handler(std::move(handler)) {}

void AsyncServer::start_accept() {
    accept();
}

void AsyncServer::accept() {
    auto self = shared_from_this();
    _acceptor.async_accept([this, self](boost::system::error_code ec, tcp::socket socket) -> void {
        std::cout << "Accepted new connection" << std::endl;
        if (!ec) {
            auto start_message_size_net_ptr = std::make_shared<std::array<char, sizeof(size_t)>>();
            auto socket_ptr = std::make_shared<tcp::socket>(std::move(socket));
            boost::asio::async_read(*socket_ptr, boost::asio::buffer(*start_message_size_net_ptr),
                [this, self, start_message_size_net_ptr, socket_ptr](boost::system::error_code ec, std::size_t /*length*/) -> void {
                if (!ec) {
                    size_t start_message_size_net = 0;
                    std::memcpy(&start_message_size_net, start_message_size_net_ptr->data(), sizeof(start_message_size_net));
                    std::cout << "Reading start message of size " << start_message_size_net << ' ' << ntoh64(start_message_size_net) << std::endl;
                    size_t start_message_size = ntoh64(start_message_size_net);
                    if (start_message_size == 0) {
                        _handler->new_connection(*socket_ptr, "");
                        return;
                    }
                    auto start_message = std::make_shared<std::string>(start_message_size, '\0');

                    boost::asio::async_read(*socket_ptr, boost::asio::buffer(*start_message),
                        [this, self, start_message, socket_ptr](boost::system::error_code ec, std::size_t /*length*/) -> void {

                        std::cout << "Read start message: " << *start_message << std::endl;
                        
                        if (!ec) {
                            _handler->new_connection(*socket_ptr, *start_message);
                        } else {
                            std::cout << "Error reading start message: " << ec.what() << std::endl;
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