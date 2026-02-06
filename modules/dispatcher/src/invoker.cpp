#include "oink_judge/dispatcher/invoker.h"

namespace oink_judge::dispatcher {

Invoker::Invoker(std::string id, std::shared_ptr<Session> session) : id_(std::move(id)), session_(std::move(session)) {
    if (!session_) {
        throw std::invalid_argument("Session cannot be null");
    }
}

auto Invoker::getId() const -> const std::string& { return id_; }

auto Invoker::sendMessage(const std::string& message) -> void {
    co_spawn(session_->getExecutor(), session_->sendMessage(message), boost::asio::detached);
}

auto Invoker::canTestSubmission(const std::string& submission_id) const -> bool {
    // Default implementation, can be overridden by derived classes
    return true; // Assuming all invokers can test submissions by default
}

} // namespace oink_judge::dispatcher
