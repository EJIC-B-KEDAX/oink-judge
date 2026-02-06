#include "oink_judge/socket/connection_protocol.h"

#include "oink_judge/socket/boost_io_context.h"
#include "oink_judge/socket/byte_order.h"
#include "oink_judge/socket/session.hpp"

#include <oink_judge/logger/logger.h>
#include <utility>

namespace oink_judge::socket {

auto asyncConnectToTheEndpoint(std::string host, short port, std::string session_type, std::string request)
    -> awaitable<std::shared_ptr<Session>> {
    auto& io_context = BoostIOContext::instance();
    boost::asio::ip::tcp::socket socket(io_context);
    tcp::resolver resolver(io_context);
    auto endpoints = resolver.resolve(host, std::to_string(port));

    try {
        co_await boost::asio::async_connect(socket, endpoints, boost::asio::use_awaitable);

        size_t message_length_net = hton64(request.size());

        co_await boost::asio::async_write(socket, boost::asio::buffer(&message_length_net, sizeof(message_length_net)),
                                          boost::asio::use_awaitable);

        co_await boost::asio::async_write(socket, boost::asio::buffer(request), boost::asio::use_awaitable);

        auto session_ptr = SessionFactory::instance().create(session_type, std::move(socket));
        co_await session_ptr->start(request);

        co_return session_ptr;

    } catch (const std::exception& e) {
        logger::logMessage("socket", 1, std::string("Error during async connect: ") + e.what(), logger::ERROR);
        socket.close();
    }

    co_return co_await asyncScheduleConnectToTheEndpoint(host, port, session_type, request);
}

auto asyncScheduleConnectToTheEndpoint(std::string host, short port, std::string session_type, std::string request)
    -> awaitable<std::shared_ptr<Session>> {
    auto& io_context = BoostIOContext::instance();

    boost::asio::steady_timer timer(io_context);
    timer.expires_after(std::chrono::seconds(1));

    try {
        co_await timer.async_wait(boost::asio::use_awaitable);
        co_return co_await asyncConnectToTheEndpoint(host, port, session_type, request);
    } catch (const std::exception& e) {
        logger::logMessage("socket", 1, std::string("Error during scheduled async connect: ") + e.what(), logger::ERROR);
    }

    co_return co_await asyncConnectToTheEndpoint(host, port, session_type, request);
}

auto connectToTheEndpoint(std::string host, short port, std::string session_type, std::string request)
    -> std::shared_ptr<Session> {
    auto& io_context = BoostIOContext::instance();
    auto socket = std::make_shared<boost::asio::ip::tcp::socket>(io_context);
    tcp::resolver resolver(io_context);
    auto endpoints = resolver.resolve(host, std::to_string(port));

    try {
        boost::asio::connect(*socket, endpoints);

        size_t message_length = hton64(request.size());
        boost::asio::write(*socket, boost::asio::buffer(&message_length, sizeof(message_length)));
        boost::asio::write(*socket, boost::asio::buffer(request));

        auto ptr = SessionFactory::instance().create(session_type, std::move(*socket));
        co_spawn(io_context, ptr->start(request), boost::asio::detached); // TODO make this synchronous

        return ptr;
    } catch (const boost::system::system_error& e) {
        logger::logMessage("socket", 1, std::string("Error during connect: ") + e.what(), logger::ERROR);
        socket->close();
        return scheduleConnectToTheEndpoint(std::move(host), port, std::move(session_type), std::move(request));
    }
}

auto scheduleConnectToTheEndpoint(std::string host, short port, std::string session_type, std::string request)
    -> std::shared_ptr<Session> {
    auto& io_context = BoostIOContext::instance();

    boost::asio::steady_timer timer(io_context, std::chrono::seconds(1)); // TODO make this configurable
    timer.wait();

    return connectToTheEndpoint(std::move(host), port, std::move(session_type), std::move(request));
}

} // namespace oink_judge::socket