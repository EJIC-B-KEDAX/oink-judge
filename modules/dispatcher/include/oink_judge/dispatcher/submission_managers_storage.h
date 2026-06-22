#pragma once
#include "oink_judge/dispatcher/problem_submission_manager.hpp"

#include <map>
#include <memory>
#include <mutex>
#include <string>

namespace oink_judge::dispatcher {

class SubmissionManagersStorage {
  public:
    static auto instance() -> SubmissionManagersStorage&;

    SubmissionManagersStorage(const SubmissionManagersStorage&) = delete;
    auto operator=(const SubmissionManagersStorage&) -> SubmissionManagersStorage& = delete;
    SubmissionManagersStorage(SubmissionManagersStorage&&) = delete;
    auto operator=(SubmissionManagersStorage&&) -> SubmissionManagersStorage& = delete;
    ~SubmissionManagersStorage() = default;

    auto getSubmissionManager(const std::string& problem_id) -> std::shared_ptr<ProblemSubmissionManager>;

  private:
    SubmissionManagersStorage();

    std::map<std::string, std::shared_ptr<ProblemSubmissionManager>> submission_managers_;
    std::mutex mutex_;
};

} // namespace oink_judge::dispatcher