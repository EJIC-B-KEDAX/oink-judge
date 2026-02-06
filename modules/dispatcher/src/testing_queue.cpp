#include "oink_judge/dispatcher/testing_queue.h"

#include <cstddef>
#include <oink_judge/logger/logger.h>

namespace oink_judge::dispatcher {

auto TestingQueue::instance() -> TestingQueue& {
    static TestingQueue instance;
    return instance;
}

auto TestingQueue::pushSubmission(const std::string& submission_id) -> void {
    if (trySubmission(submission_id)) {
        return;
    }

    submission_queue_.push_back(submission_id);
}

auto TestingQueue::freeInvoker(const std::string& invoker_id) -> void {
    if (tryInvoker(findInvokerById(invoker_id))) {
        return;
    }

    connected_invokers_[findInvokerIndexById(invoker_id)].second = "";
}

auto TestingQueue::connectInvoker(std::unique_ptr<Invoker> invoker_ptr) -> void {
    connected_invokers_.emplace_back(std::move(invoker_ptr), "");

    tryInvoker(*connected_invokers_.back().first);
}

auto TestingQueue::disconnectInvoker(const std::string& invoker_id) -> void {
    size_t index = findInvokerIndexById(invoker_id);

    if (!connected_invokers_[index].second.empty()) {
        submission_queue_.insert(submission_queue_.begin(), connected_invokers_[index].second);
    }

    connected_invokers_.erase(connected_invokers_.begin() + static_cast<long>(index));
}

TestingQueue::TestingQueue() = default;

auto TestingQueue::tryInvoker(Invoker& invoker) -> bool {
    for (int i = 0; i < submission_queue_.size(); i++) {
        if (invoker.canTestSubmission(submission_queue_[i])) {
            sendSubmissionForTesting(submission_queue_[i], invoker);
            submission_queue_.erase(submission_queue_.begin() + i);
            return true;
        }
    }

    return false;
}

auto TestingQueue::trySubmission(const std::string& submission_id) -> bool {
    for (auto& [invoker_ptr, busy_with] : connected_invokers_) {
        if (busy_with.empty() && invoker_ptr->canTestSubmission(submission_id)) {
            sendSubmissionForTesting(submission_id, *invoker_ptr);

            return true;
        }
    }

    return false;
}

auto TestingQueue::sendSubmissionForTesting(const std::string& submission_id, Invoker& invoker) -> void {
    connected_invokers_[findInvokerIndexById(invoker.getId())].second = submission_id;
    std::string message = R"({"request": "test_submission","submission_id": ")" + submission_id + "\"}";
    invoker.sendMessage(message);
}

auto TestingQueue::findInvokerIndexById(const std::string& invoker_id) const -> size_t {
    for (size_t i = 0; i < connected_invokers_.size(); i++) {
        if (connected_invokers_[i].first->getId() == invoker_id) {
            return i;
        }
    }

    logger::logMessage("TestingQueue", 1, "Invoker with ID " + invoker_id + " not found.", logger::LogType::ERROR);
    throw std::runtime_error("Invoker with ID " + invoker_id + " not found.");
}

auto TestingQueue::findInvokerById(const std::string& invoker_id) -> Invoker& {
    size_t index = findInvokerIndexById(invoker_id);
    return *connected_invokers_[index].first;
}

} // namespace oink_judge::dispatcher
