#include "socket/protocols/PingingProtocol.h"
#include "socket/BoostIOContext.h"
#include "config/Config.h"
#include <iostream>

namespace oink_judge::socket {

namespace {

[[maybe_unused]] bool registered = []() {
    ProtocolFactory::instance().register_type(PingingProtocol::REGISTERED_NAME,
        [](const std::string &params) -> std::unique_ptr<Protocol> {
        std::string inner_type = params;

        return std::make_unique<PingingProtocol>(ProtocolFactory::instance().create(inner_type));
    });
    return true;
}();

} // namespace

PingingProtocol::PingingProtocol(std::unique_ptr<Protocol> inner_protocol)
    : _inner_protocol(std::move(inner_protocol)), _ping_timer(BoostIOContext::instance()), _pong_timer(BoostIOContext::instance()) {
    const auto &config = config::Config::config();
    _ping_interval_seconds = config["bounds"]["ping_interval"].get<float>();
    _pong_timeout_seconds = config["bounds"]["pong_timeout"].get<float>();
    }

void PingingProtocol::start(const std::string &start_message) {
    _inner_protocol->start(start_message);
    ping_loop();
}

void PingingProtocol::receive_message(const std::string &message) {
    if (message == "pong") {
        _pong_timer.cancel();
        get_session()->receive_message();
        return;
    }
    _inner_protocol->receive_message(message);
}

void PingingProtocol::close_session() {
    _ping_timer.cancel();
    _pong_timer.cancel();
    _inner_protocol->close_session();
}

void PingingProtocol::set_session(std::weak_ptr<Session> session) {
    _inner_protocol->set_session(session);
}

std::shared_ptr<Session> PingingProtocol::get_session() const {
    return _inner_protocol->get_session();
}

void PingingProtocol::request_internal(const std::string &message, const callback_t &callback) {
    _inner_protocol->request_internal(message, callback);
}

void PingingProtocol::ping_loop() {
    auto session = get_session();

    _ping_timer.expires_after(std::chrono::milliseconds(static_cast<int64_t>(_ping_interval_seconds * 1000)));
    _ping_timer.async_wait([this, session](const boost::system::error_code &ec) {
        if (ec) {
            return;
        }

        session->send_message("ping");
        wait_for_pong();
        ping_loop();
    });
}

void PingingProtocol::wait_for_pong() {
    auto session = get_session();

    _pong_timer.expires_after(std::chrono::milliseconds(static_cast<int64_t>(_pong_timeout_seconds * 1000)));
    _pong_timer.async_wait([this, session](const boost::system::error_code &ec) {
        if (ec) {
            return;
        }

        session->close();
    });
}

} // namespace oink_judge::socket
