#include "oink_judge/socket/protocols/pinging_protocol.h"

#include <oink_judge/config/common_utils.h>
#include <oink_judge/logger/logger.h>

#include <boost/asio/any_io_executor.hpp>

#include <chrono>

namespace oink_judge::socket {

namespace {

const int SECONDS_TO_MILLISECONDS = 1000;

} // namespace

PingingProtocol::PingingProtocol(std::unique_ptr<Protocol> inner_protocol, const boost::asio::any_io_executor& executor)
    : ProtocolDecorator(std::move(inner_protocol)), ping_timer_(executor), pong_timer_(executor), ping_interval_seconds_(),
      pong_timeout_seconds_() {
    auto ping_interval_opt = config::getTiming("ping_interval");
    if (!ping_interval_opt.has_value()) {
        logger::logMessage("PingingProtocol", "ping_interval not set in config", logger::ERROR);
        throw std::runtime_error("ping_interval not set in config");
    }
    ping_interval_seconds_ = ping_interval_opt.value();
    auto pong_timeout_opt = config::getTiming("pong_timeout");
    if (!pong_timeout_opt.has_value()) {
        logger::logMessage("PingingProtocol", "pong_timeout not set in config", logger::ERROR);
        throw std::runtime_error("pong_timeout not set in config");
    }
    pong_timeout_seconds_ = pong_timeout_opt.value();
}

auto PingingProtocol::start(std::string start_message) -> awaitable<void> {
    co_await ProtocolDecorator::start(start_message);
    pingLoop();
}

auto PingingProtocol::receiveMessage(std::string message) -> awaitable<void> {
    if (message == "pong") {
        co_spawn(co_await boost::asio::this_coro::executor, getSession()->receiveMessage(), boost::asio::detached);
        pong_timer_.cancel();
        co_return;
    }
    co_await ProtocolDecorator::receiveMessage(message);
}

void PingingProtocol::closeSession() {
    ping_timer_.cancel();
    pong_timer_.cancel();
    ProtocolDecorator::closeSession();
}

void PingingProtocol::pingLoop() {
    auto session = getSession();

    ping_timer_.expires_after(
        std::chrono::milliseconds(static_cast<int64_t>(ping_interval_seconds_.count() * SECONDS_TO_MILLISECONDS)));
    ping_timer_.async_wait([this, session](const boost::system::error_code& ec) -> void {
        if (ec) {
            return;
        }

        co_spawn(ping_timer_.get_executor(), session->sendMessage("ping"), boost::asio::detached);
        waitForPong();
        pingLoop();
    });
}

void PingingProtocol::waitForPong() {
    auto session = getSession();

    pong_timer_.expires_after(
        std::chrono::milliseconds(static_cast<int64_t>(pong_timeout_seconds_.count() * SECONDS_TO_MILLISECONDS)));
    pong_timer_.async_wait([this, session](const boost::system::error_code& ec) -> void {
        if (ec) {
            return;
        }

        session->close();
    });
}

auto registerPingingProtocolType() -> void {
    ProtocolFactory::instance().registerType(
        PingingProtocol::REGISTERED_NAME,
        [](const std::string& params, const boost::asio::any_io_executor& executor) -> std::unique_ptr<Protocol> {
            const std::string& inner_type = params;

            return std::make_unique<PingingProtocol>(
                ProtocolFactory::instance().create(inner_type, boost::asio::any_io_executor(executor)), executor);
        });
}

} // namespace oink_judge::socket
