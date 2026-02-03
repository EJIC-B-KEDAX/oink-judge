#include "oink_judge/socket/connection_protocol.h"

#include "oink_judge/socket/boost_io_context.h"
#include "oink_judge/socket/byte_order.h"
#include "oink_judge/socket/session.hpp"

#include <iostream>

namespace oink_judge::socket {

awaitable<std::shared_ptr<Session>> async_connect_to_the_endpoint(std::string host, short port, std::string session_type,
                                                                  std::string request) {
    auto& io_context = BoostIOContext::instance();
    boost::asio::ip::tcp::socket socket(io_context);
    tcp::resolver resolver(io_context);
    auto endpoints = resolver.resolve(host, std::to_string(port));
    std::cerr << "Start message: " << request << std::endl;

    try {
        co_await boost::asio::async_connect(socket, endpoints, boost::asio::use_awaitable);

        std::cerr << "Connected to " << host << ":" << port << std::endl;
        std::cerr << "Sending start message: " << request << std::endl;

        size_t message_length_net = hton64(request.size());

        co_await boost::asio::async_write(socket, boost::asio::buffer(&message_length_net, sizeof(message_length_net)),
                                          boost::asio::use_awaitable);

        co_await boost::asio::async_write(socket, boost::asio::buffer(request), boost::asio::use_awaitable);

        auto session_ptr = SessionFactory::instance().create(session_type, std::move(socket));
        co_await session_ptr->start(request);

        co_return session_ptr;

    } catch (const std::exception& e) {
        socket.close();
        std::cerr << "Error during async connect: " << e.what() << std::endl;
    }

    co_return co_await async_schedule_connect_to_the_endpoint(host, port, session_type, request);
}

awaitable<std::shared_ptr<Session>> async_schedule_connect_to_the_endpoint(std::string host, short port, std::string session_type,
                                                                           std::string request) {
    auto& io_context = BoostIOContext::instance();

    boost::asio::steady_timer timer(io_context);
    timer.expires_after(std::chrono::seconds(1));

    try {
        co_await timer.async_wait(boost::asio::use_awaitable);
        co_return co_await async_connect_to_the_endpoint(host, port, session_type, request);
    } catch (const std::exception& e) {
        std::cerr << "Error during scheduled connect: " << e.what() << std::endl;
    }

    co_return co_await async_connect_to_the_endpoint(host, port, session_type, request);
}

std::shared_ptr<Session> connect_to_the_endpoint(std::string host, short port, std::string session_type, std::string request) {
    auto& io_context = BoostIOContext::instance();
    auto socket = std::make_shared<boost::asio::ip::tcp::socket>(io_context);
    tcp::resolver resolver(io_context);
    auto endpoints = resolver.resolve(host, std::to_string(port));

    std::cout << "Connecting to " << host << ":" << port << std::endl;
    std::cout << "Using session type: " << session_type << std::endl;

    try {
        boost::asio::connect(*socket, endpoints);

        size_t message_length = hton64(request.size());
        boost::asio::write(*socket, boost::asio::buffer(&message_length, sizeof(message_length)));
        boost::asio::write(*socket, boost::asio::buffer(request));

        auto ptr = SessionFactory::instance().create(session_type, std::move(*socket));
        co_spawn(io_context, ptr->start(request), boost::asio::detached); // TODO make this synchronous

        return ptr;
    } catch (const boost::system::system_error& e) {
        socket->close();
        return schedule_connect_to_the_endpoint(host, port, session_type, request);
    }
}

std::shared_ptr<Session> schedule_connect_to_the_endpoint(std::string host, short port, std::string session_type,
                                                          std::string request) {
    auto& io_context = BoostIOContext::instance();

    boost::asio::steady_timer timer(io_context, std::chrono::seconds(1)); // TODO make this configurable
    timer.wait();

    return connect_to_the_endpoint(host, port, session_type, request);
}

} // namespace oink_judge::socket