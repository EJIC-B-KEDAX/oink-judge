#include "oink_judge/socket/connection_protocol.h"

#include "oink_judge/socket/byte_order.h"
#include "oink_judge/socket/session.hpp"

#include <oink_judge/logger/logger.h>

#include <future>
#include <utility>

namespace oink_judge::socket {

auto asyncConnectToTheEndpoint(std::string host, short port, std::string session_type, std::string request)
    -> awaitable<std::shared_ptr<Session>> {
    auto context = co_await boost::asio::this_coro::executor;
    boost::asio::ip::tcp::socket socket(context);
    tcp::resolver resolver(context);
    auto endpoints = resolver.resolve(host, std::to_string(port));

    try {
        co_await boost::asio::async_connect(socket, endpoints, boost::asio::use_awaitable);

        size_t message_length_net = hton64(request.size());

        co_await boost::asio::async_write(socket, boost::asio::buffer(&message_length_net, sizeof(message_length_net)),
                                          boost::asio::use_awaitable);

        co_await boost::asio::async_write(socket, boost::asio::buffer(request), boost::asio::use_awaitable);

        auto session_ptr = SessionFactory::instance().create(session_type, std::move(socket));
        logger::logInfo("socket",
                        "Successfully connected to " + host + ":" + std::to_string(port) + " with session type: " + session_type);
        co_await session_ptr->start(request);
        logger::logInfo("socket", "Session started successfully for " + host + ":" + std::to_string(port));

        co_return session_ptr;

    } catch (const std::exception& e) {
        logger::logError("socket", std::string("Error during async connect: ") + e.what());
        socket.close();
    }

    co_return co_await asyncScheduleConnectToTheEndpoint(host, port, session_type, request);
}

auto asyncScheduleConnectToTheEndpoint(std::string host, short port, std::string session_type, std::string request)
    -> awaitable<std::shared_ptr<Session>> {

    auto context = co_await boost::asio::this_coro::executor;

    boost::asio::steady_timer timer(context);
    timer.expires_after(std::chrono::seconds(1));

    logger::logInfo("socket",
                    "Scheduling async connect to " + host + ":" + std::to_string(port) + " with session type: " + session_type);

    try {
        co_await timer.async_wait(boost::asio::use_awaitable);
        logger::logInfo("socket", "Timer expired, attempting to reconnect to " + host + ":" + std::to_string(port));
        co_return co_await asyncConnectToTheEndpoint(host, port, session_type, request);
    } catch (const std::exception& e) {
        logger::logError("socket", std::string("Error during scheduled async connect: ") + e.what());
    }

    co_return co_await asyncConnectToTheEndpoint(host, port, session_type, request);
}

auto connectToTheEndpoint(std::string host, short port, std::string session_type, std::string request,
                          boost::asio::io_context& io_context) -> std::shared_ptr<Session> {
    auto socket = std::make_shared<boost::asio::ip::tcp::socket>(io_context);
    tcp::resolver resolver(io_context);
    auto endpoints = resolver.resolve(host, std::to_string(port));

    try {
        boost::asio::connect(*socket, endpoints);

        size_t message_length = hton64(request.size());
        boost::asio::write(*socket, boost::asio::buffer(&message_length, sizeof(message_length)));
        boost::asio::write(*socket, boost::asio::buffer(request));

        auto ptr = SessionFactory::instance().create(session_type, std::move(*socket));

        // Start the session and wait for it to finish starting before returning.
        // Use co_spawn with use_future and block on the returned std::future.

        logger::logInfo("socket",
                        "Successfully connected to " + host + ":" + std::to_string(port) + " with session type: " + session_type);

        auto fut = boost::asio::co_spawn(
            io_context,
            [ptr, request]() -> awaitable<void> { // NOLINT
                co_await ptr->start(request);
                co_return;
            },
            boost::asio::use_future);

        logger::logInfo("socket", "Wait for session start for " + host + ":" + std::to_string(port));

        while (fut.wait_for(std::chrono::seconds(0)) != std::future_status::ready) {
            io_context.run_one(); // обработать 1 событие
        }

        fut.get();

        logger::logInfo("socket", "Session started successfully for " + host + ":" + std::to_string(port));

        return ptr;
    } catch (const boost::system::system_error& e) {
        logger::logError("socket", std::string("Error during connect: ") + e.what());
        socket->close();
        return scheduleConnectToTheEndpoint(std::move(host), port, std::move(session_type), std::move(request), io_context);
    }
}

auto scheduleConnectToTheEndpoint(std::string host, short port, std::string session_type, std::string request,
                                  boost::asio::io_context& io_context) -> std::shared_ptr<Session> {

    boost::asio::steady_timer timer(io_context, std::chrono::seconds(1)); // TODO make this configurable
    timer.wait();

    return connectToTheEndpoint(std::move(host), port, std::move(session_type), std::move(request), io_context);
}

} // namespace oink_judge::socket
