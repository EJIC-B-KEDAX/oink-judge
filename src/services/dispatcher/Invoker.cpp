#include "services/dispatcher/Invoker.h"

namespace oink_judge::services::dispatcher {

Invoker::Invoker(std::string identificator, std::shared_ptr<Session> session)
    : _id(std::move(identificator)), _session(std::move(session)) {
    if (!_session) {
        throw std::invalid_argument("Session cannot be null");
    }
}

auto Invoker::get_id() const -> const std::string& { return _id; }

auto Invoker::send_message(const std::string& message) -> void {
    co_spawn(_session->get_executor(), _session->send_message(message), boost::asio::detached);
}

auto Invoker::can_test_submission(const std::string& submission_id) const -> bool {
    // Default implementation, can be overridden by derived classes
    return true; // Assuming all invokers can test submissions by default
}

} // namespace oink_judge::services::dispatcher
