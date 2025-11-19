#include "services/dispatcher/DefaultSessionWithInvokerEventHandler.h"
#include "services/dispatcher/TestingQueue.h"
#include <nlohmann/json.hpp>
#include "config/Config.h"

namespace oink_judge::services::dispatcher {

namespace {

[[maybe_unused]] bool registered = []() -> bool {
    socket::BasicSessionEventHandlerFactory::instance().register_type(DefaultSessionWithInvokerEventHandler::REGISTERED_NAME,
        [](const std::string &params) -> std::unique_ptr<DefaultSessionWithInvokerEventHandler> {
        return std::make_unique<DefaultSessionWithInvokerEventHandler>();
    });

    return true;
}();

} // namespace

using json = nlohmann::json;

DefaultSessionWithInvokerEventHandler::DefaultSessionWithInvokerEventHandler() = default;

void DefaultSessionWithInvokerEventHandler::start(const std::string &start_message) {
    get_session()->receive_message();
}

void DefaultSessionWithInvokerEventHandler::receive_message(const std::string &message) {
    if (_invoker_id.empty()) {
        json response = json::parse(message);
        _invoker_id = response["invoker_id"];
        
        if (_invoker_id.empty()) {
            get_session()->receive_message();
            return;
        }

        auto invoker = std::make_unique<Invoker>(_invoker_id, get_session());
        TestingQueue::instance().connect_invoker(std::move(invoker));

        get_session()->receive_message();
    } else {
        if (message == "I am free") {
            TestingQueue::instance().free_invoker(_invoker_id);
        }

        _session.lock()->receive_message();
    }
}

void DefaultSessionWithInvokerEventHandler::close_session() {
    if (_invoker_id.empty()) return;

    TestingQueue::instance().disconnect_invoker(_invoker_id);
    _invoker_id.clear();
}

void DefaultSessionWithInvokerEventHandler::set_session(std::weak_ptr<socket::Session> session) {
    _session = session;
}

std::shared_ptr<Session> DefaultSessionWithInvokerEventHandler::get_session() const {
    return _session.lock();
}

void DefaultSessionWithInvokerEventHandler::request_internal(const std::string &message, const callback_t &callback) {
    throw std::runtime_error("Request not supported in DefaultSessionWithInvokerEventHandler");
}

} // namespace oink_judge::services::dispatcher
