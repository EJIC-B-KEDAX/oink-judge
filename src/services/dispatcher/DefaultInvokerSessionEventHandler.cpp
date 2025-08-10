#include "services/dispatcher/DefaultInvokerSessionEventHandler.h"
#include "config/Config.h"

namespace oink_judge::services::dispatcher {

using json = nlohmann::json;

namespace {

[[maybe_unused]] bool registered = []() -> bool {
    socket::BasicSessionEventHandlerFactory::instance().register_type(DefaultInvokerSessionEventHandler::REGISTERED_NAME,
        [](const std::string &params) -> std::unique_ptr<DefaultInvokerSessionEventHandler> {
        return std::make_unique<DefaultInvokerSessionEventHandler>();
    });

    return true;
}();

} // namespace

DefaultInvokerSessionEventHandler::DefaultInvokerSessionEventHandler() = default;

void DefaultInvokerSessionEventHandler::start(const std::string &start_message) {
    json set_id_request = {
        {"invker_id", config::Config::config()["my_id"].get<std::string>()}
    };
    get_session().lock()->send_message(set_id_request.dump());
    get_session().lock()->receive_message();
}

void DefaultInvokerSessionEventHandler::receive_message(const std::string &message) {
    json parsed_message = json::parse(message);

    if (parsed_message["request"] == "test_submission")  {
        // Handle submission-related messages
    }

    get_session().lock()->send_message("I am free");
    get_session().lock()->receive_message();
}

void DefaultInvokerSessionEventHandler::close_session() {
    // reconnect to the server
}

void DefaultInvokerSessionEventHandler::set_session(std::weak_ptr<socket::Session> session) {
    _session = session;
}

std::weak_ptr<socket::Session> DefaultInvokerSessionEventHandler::get_session() const {
    return _session;
}

} // namespace oink_judge::services::dispatcher
