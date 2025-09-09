#include "socket/connection_protocol.h"
#include "config/Config.h"
#include "socket/BoostIOContext.h"
#include "socket/Session.hpp"
#include "socket/byte_order.h"
#include <iostream>

namespace oink_judge::socket {

void async_connect_to_the_endpoint(const std::string &host, short port, const std::string &session_type, const std::string &request) {
    auto& io_context = BoostIOContext::instance();
    auto socket = std::make_shared<boost::asio::ip::tcp::socket>(io_context);
    tcp::resolver resolver(io_context);
    auto endpoints = resolver.resolve(host, std::to_string(port));
    auto request_ptr = std::make_shared<std::string>(request);

    boost::asio::async_connect(*socket, endpoints, 
        [socket, request_ptr, host, port, session_type](boost::system::error_code ec, tcp::endpoint) {
        if (!ec) {
            size_t message_length = hton64(request_ptr->size());
            std::cout << "Connected to " << host << ":" << port << std::endl;
            std::cout << "Sending request: " << *request_ptr << std::endl;
            std::cout << "Request length: " << request_ptr->size() << ' ' << " in network " << message_length << " must be " << ntoh64(message_length) << std::endl;
            auto length_buf = std::make_shared<std::array<char, sizeof(message_length)>>();
            std::memcpy(length_buf->data(), &message_length, sizeof(message_length));
            boost::asio::async_write(*socket, boost::asio::buffer(*length_buf),
                [socket, length_buf, request_ptr, host, port, session_type](const boost::system::error_code &ec, std::size_t /*length*/) -> void {
                std::cout << "Sent length of the request: " << request_ptr->size() << std::endl;
                size_t message_length = 0;
                std::memcpy(&message_length, length_buf->data(), sizeof(message_length));
                std::cout << "Length buffer contains: " << message_length << " in host " << ntoh64(message_length) << std::endl;
                if (!ec) {
                    boost::asio::async_write(*socket, boost::asio::buffer(*request_ptr),
                        [socket, request_ptr, host, port, session_type](const boost::system::error_code &ec, std::size_t /*length*/) -> void {
                        if (!ec) {
                            BasicSessionFactory::instance().create(session_type, std::move(*socket))->start(*request_ptr);
                        } else {
                            socket->close();
                            async_schedule_connect_to_the_endpoint(host, port, session_type, *request_ptr);
                        }
                    });
                } else {
                    socket->close();
                    async_schedule_connect_to_the_endpoint(host, port, session_type, *request_ptr);
                }
            });
        } else {
            socket->close();
            async_schedule_connect_to_the_endpoint(host, port, session_type, *request_ptr);
        }
    });
}

void async_schedule_connect_to_the_endpoint(const std::string &host, short port, const std::string &session_type, const std::string &request) {
    auto& io_context = BoostIOContext::instance();

    boost::asio::steady_timer timer(io_context);
    timer.expires_after(std::chrono::seconds(1)); // TODO make this configurable
    timer.async_wait([host, port, session_type, request](const boost::system::error_code& ec) {
        if (ec) return;
        async_connect_to_the_endpoint(host, port, session_type, request);
    });
}

std::shared_ptr<Session> connect_to_the_endpoint(const std::string &host, short port, const std::string &session_type, const std::string &request) {
    auto& io_context = BoostIOContext::instance();
    auto socket = std::make_shared<boost::asio::ip::tcp::socket>(io_context);
    tcp::resolver resolver(io_context);
    auto endpoints = resolver.resolve(host, std::to_string(port));

    try {
        boost::asio::connect(*socket, endpoints);

        size_t message_length = hton64(request.size());
        boost::asio::write(*socket, boost::asio::buffer(&message_length, sizeof(message_length)));
        boost::asio::write(*socket, boost::asio::buffer(request));

        auto ptr = BasicSessionFactory::instance().create(session_type, std::move(*socket));
        ptr->start(request);

        return ptr;
    } catch (const boost::system::system_error &e) {
        socket->close();
        return schedule_connect_to_the_endpoint(host, port, session_type, request);
    }
}

std::shared_ptr<Session> schedule_connect_to_the_endpoint(const std::string &host, short port, const std::string &session_type, const std::string &request) {
    auto& io_context = BoostIOContext::instance();

    boost::asio::steady_timer timer(io_context, std::chrono::seconds(1)); // TODO make this configurable
    timer.wait();

    return connect_to_the_endpoint(host, port, session_type, request);
}

}