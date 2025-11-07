#include "socket/session_event_handlers/PingingSessionEventHandler.h"
#include "socket/BoostIOContext.h"
#include "config/Config.h"

namespace oink_judge::socket {

namespace {

[[maybe_unused]] bool registered = []() {
    BasicSessionEventHandlerFactory::instance().register_type(PingingSessionEventHandler::REGISTERED_NAME,
        [](const std::string &params) -> std::unique_ptr<SessionEventHandler> {
        std::string inner_type = params;

        return std::make_unique<PingingSessionEventHandler>(BasicSessionEventHandlerFactory::instance().create(inner_type));
    });
    return true;
}();

} // namespace

PingingSessionEventHandler::PingingSessionEventHandler(std::unique_ptr<SessionEventHandler> inner_event_handler)
    : _inner_event_handler(std::move(inner_event_handler)), _ping_timer(BoostIOContext::instance()), _pong_timer(BoostIOContext::instance()) {
    const auto &config = config::Config::config();
    _ping_interval_seconds = config["bounds"]["ping_interval"].get<float>();
    _pong_timeout_seconds = config["bounds"]["pong_timeout"].get<float>();
    }

void PingingSessionEventHandler::start(const std::string &start_message) {
    _inner_event_handler->start(start_message);
    ping_loop();
}

void PingingSessionEventHandler::receive_message(const std::string &message) {
    if (message == "pong") {
        _pong_timer.cancel();
        get_session().lock()->receive_message();
        return;
    }
    _inner_event_handler->receive_message(message);
}

void PingingSessionEventHandler::close_session() {
    _inner_event_handler->close_session();
}

void PingingSessionEventHandler::set_session(std::weak_ptr<Session> session) {
    _inner_event_handler->set_session(session);
}

std::weak_ptr<Session> PingingSessionEventHandler::get_session() const {
    return _inner_event_handler->get_session();
}

void PingingSessionEventHandler::ping_loop() {
    auto session = get_session().lock();

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

void PingingSessionEventHandler::wait_for_pong() {
    auto session = get_session().lock();

    _pong_timer.expires_after(std::chrono::milliseconds(static_cast<int64_t>(_pong_timeout_seconds * 1000)));
    _pong_timer.async_wait([this, session](const boost::system::error_code &ec) {
        if (ec) {
            return;
        }
        
        session->close();
    });
}

} // namespace oink_judge::socket
