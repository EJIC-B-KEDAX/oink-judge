#include "services/dispatcher/Invoker.h"

namespace oink_judge::services::dispatcher {

Invoker::Invoker(std::string id, std::shared_ptr<Session> session) : _id(std::move(id)), _session(std::move(session)) {
    if (!_session) {
        throw std::invalid_argument("Session cannot be null");
    }
}

const std::string &Invoker::get_id() const {
    return _id;
}

void Invoker::send_message(const std::string &message) {
    _session->send_message(message);
}

bool Invoker::can_test_submission(const std::string &submission_id) const {
    // Default implementation, can be overridden by derived classes
    return true; // Assuming all invokers can test submissions by default
}

} // namespace oink_judge::services::dispatcher
