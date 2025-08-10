#include "socket/connection_protocol.h"
#include "config/Config.h"
#include "socket/BoostIOContext.h"
#include "socket/Session.hpp"
#include "socket/byte_order.h"

namespace oink_judge::socket {

void async_connect_to_the_endpoint(const std::string &host, short port, const std::string &session_type, const std::string &request) {
    auto& io_context = BoostIOContext::instance();
    auto socket = std::make_shared<boost::asio::ip::tcp::socket>(io_context);
    tcp::resolver resolver(io_context);
    auto endpoints = resolver.resolve(host, std::to_string(port));

    boost::asio::async_connect(*socket, endpoints, 
        [socket, request, host, port, session_type](boost::system::error_code ec, tcp::endpoint) {
        if (!ec) {
            size_t message_length = hton64(request.size());
            boost::asio::async_write(*socket, boost::asio::buffer(&message_length, sizeof(message_length)),
                [socket, request, host, port, session_type](const boost::system::error_code &ec, std::size_t /*length*/) -> void {
                if (!ec) {
                    boost::asio::async_write(*socket, boost::asio::buffer(request),
                        [socket, request, host, port, session_type](const boost::system::error_code &ec, std::size_t /*length*/) -> void {
                        if (!ec) {
                            BasicSessionFactory::instance().create(session_type, std::move(*socket))->start(request);
                        } else {
                            socket->close();
                            async_schedule_connect_to_the_endpoint(host, port, session_type, request);
                        }
                    });
                } else {
                    socket->close();
                    async_schedule_connect_to_the_endpoint(host, port, session_type, request);
                }
            });
        } else {
            socket->close();
            async_schedule_connect_to_the_endpoint(host, port, session_type, request);
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

        return BasicSessionFactory::instance().create(session_type, std::move(*socket));
    } catch (const boost::system::system_error &e) {
        socket->close();
        return schedule_connect_to_the_endpoint(host, port, session_type, request);
    }
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

        std::shared_ptr<Session> session = BasicSessionFactory::instance().create(session_type, std::move(*socket));

        session->start(request);

        return session;
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