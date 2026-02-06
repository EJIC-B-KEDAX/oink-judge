#pragma once
#include "oink_judge/dispatcher/invoker.h"

#include <string>
#include <vector>

namespace oink_judge::dispatcher {

class TestingQueue {
  public:
    static auto instance() -> TestingQueue&;

    TestingQueue(const TestingQueue&) = delete;
    auto operator=(const TestingQueue&) -> TestingQueue& = delete;
    TestingQueue(TestingQueue&&) = delete;
    auto operator=(TestingQueue&&) -> TestingQueue& = delete;
    ~TestingQueue() = default;

    void pushSubmission(const std::string& submission_id);

    void freeInvoker(const std::string& invoker_id);

    void connectInvoker(std::unique_ptr<Invoker> invoker_ptr);

    void disconnectInvoker(const std::string& invoker_id);

  private:
    TestingQueue();

    auto tryInvoker(Invoker& invoker) -> bool;
    auto trySubmission(const std::string& submission_id) -> bool;
    void sendSubmissionForTesting(const std::string& submission_id, Invoker& invoker);
    [[nodiscard]] auto findInvokerIndexById(const std::string& invoker_id) const -> size_t;
    auto findInvokerById(const std::string& invoker_id) -> Invoker&;

    std::vector<std::string> submission_queue_;
    std::vector<std::pair<std::unique_ptr<Invoker>, std::string>> connected_invokers_;
};

} // namespace oink_judge::dispatcher
