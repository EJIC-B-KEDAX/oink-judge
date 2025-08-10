#include "services/dispatcher/TestingQueue.h"

namespace oink_judge::services::dispatcher {

TestingQueue &TestingQueue::instance() {
    static TestingQueue instance;
    return instance;
}

void TestingQueue::push_submission(const std::string &submission_id) {
    if (_try_submission(submission_id)) {
        return;
    }

    _submission_queue.push_back(submission_id);
}

void TestingQueue::free_invoker(const std::string &invoker_id) {
    if (_try_invoker(_find_invoker_by_id(invoker_id))) {
        return;
    }

    _connected_invokers[_find_invoker_index_by_id(invoker_id)].second = "";
}

void TestingQueue::connect_invoker(std::unique_ptr<Invoker> invoker_ptr) {
    _connected_invokers.emplace_back(std::move(invoker_ptr), "");

    _try_invoker(*_connected_invokers.back().first);
}

void TestingQueue::disconnect_invoker(const std::string &invoker_id) {
    const size_t index = _find_invoker_index_by_id(invoker_id);

    if (!_connected_invokers[index].second.empty()) {
        _submission_queue.insert(_submission_queue.begin(), _connected_invokers[index].second);
    }

    _connected_invokers.erase(_connected_invokers.begin() + static_cast<long>(index));
}

TestingQueue::TestingQueue() = default;

bool TestingQueue::_try_invoker(Invoker &invoker) {
    for (int i = 0; i < _submission_queue.size(); i++) {
        if (invoker.can_test_submission(_submission_queue[i])) {
            _send_submission_for_testing(_submission_queue[i], invoker);
            _submission_queue.erase(_submission_queue.begin() + i);

            return true;
        }
    }

    return false;
}

bool TestingQueue::_try_submission(const std::string &submission_id) {
    for (auto &[invoker_ptr, busy_with] : _connected_invokers) {
        if (busy_with.empty() && invoker_ptr->can_test_submission(submission_id)) {
            _send_submission_for_testing(submission_id, *invoker_ptr);

            return true;
        }
    }

    return false;
}

void TestingQueue::_send_submission_for_testing(const std::string &submission_id, Invoker &invoker) {
    _connected_invokers[_find_invoker_index_by_id(invoker.get_id())].second = submission_id;
    const std::string message = R"({"request": "test_submission","submission_id": ")" + submission_id + "\"}";
    invoker.send_message(message);
}

size_t TestingQueue::_find_invoker_index_by_id(const std::string &invoker_id) const {
    for (size_t i = 0; i < _connected_invokers.size(); i++) {
        if (_connected_invokers[i].first->get_id() == invoker_id) {
            return i;
        }
    }

    throw std::runtime_error("Invoker with ID " + invoker_id + " not found.");
}

Invoker &TestingQueue::_find_invoker_by_id(const std::string &invoker_id) {
    const size_t index = _find_invoker_index_by_id(invoker_id);
    return *_connected_invokers[index].first;
}

} // namespace oink_judge::services::dispatcher
