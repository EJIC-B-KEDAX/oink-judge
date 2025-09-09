#pragma once

#include <string>
#include <vector>
#include "Invoker.h"

namespace oink_judge::services::dispatcher {

class TestingQueue {
public:
    static TestingQueue& instance();

    TestingQueue(const TestingQueue&) = delete;
    TestingQueue& operator=(const TestingQueue&) = delete;

    void push_submission(const std::string &submission_id);

    void free_invoker(const std::string &invoker_id);

    void connect_invoker(std::unique_ptr<Invoker> invoker_ptr);

    void disconnect_invoker(const std::string &invoker_id);

private:
    TestingQueue();

    bool _try_invoker(Invoker &invoker);
    bool _try_submission(const std::string &submission_id);
    void _send_submission_for_testing(const std::string &submission_id, Invoker &invoker);
    [[nodiscard]] size_t _find_invoker_index_by_id(const std::string &invoker_id) const;
    Invoker &_find_invoker_by_id(const std::string &invoker_id);

    std::vector<std::string> _submission_queue;
    std::vector<std::pair<std::unique_ptr<Invoker>, std::string>> _connected_invokers;
};

} // namespace oink_judge::services::dispatcher
