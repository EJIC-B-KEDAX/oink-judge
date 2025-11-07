#include "socket/session_event_handlers/PongingSessionEventHandler.h"
#include "config/Config.h"

namespace oink_judge::socket {

namespace {

[[maybe_unused]] bool registered = []() {
    BasicSessionEventHandlerFactory::instance().register_type(PongingSessionEventHandler::REGISTERED_NAME,
        [](const std::string &params) -> std::unique_ptr<SessionEventHandler> {
        std::string inner_type = params;

        return std::make_unique<PongingSessionEventHandler>(BasicSessionEventHandlerFactory::instance().create(inner_type));
    });
    return true;
}();

} // namespace

PongingSessionEventHandler::PongingSessionEventHandler(std::unique_ptr<SessionEventHandler> inner_event_handler)
    : _inner_event_handler(std::move(inner_event_handler)) {}

void PongingSessionEventHandler::start(const std::string &start_message) {
    _inner_event_handler->start(start_message);
}

void PongingSessionEventHandler::receive_message(const std::string &message) {
    if (message == "ping") {
        get_session().lock()->send_message("pong");
        get_session().lock()->receive_message();
        return;
    }

    _inner_event_handler->receive_message(message);
}

void PongingSessionEventHandler::close_session() {
    _inner_event_handler->close_session();
}

void PongingSessionEventHandler::set_session(std::weak_ptr<Session> session) {
    _inner_event_handler->set_session(session);
}

std::weak_ptr<Session> PongingSessionEventHandler::get_session() const {
    return _inner_event_handler->get_session();
}

} // namespace oink_judge::socket
