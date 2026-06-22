#include "oink_judge/dispatcher/submission_managers_storage.h"

#include <oink_judge/logger/logger.h>

namespace oink_judge::dispatcher {

auto SubmissionManagersStorage::instance() -> SubmissionManagersStorage& {
    static SubmissionManagersStorage instance;

    return instance;
}

SubmissionManagersStorage::SubmissionManagersStorage() = default;

auto SubmissionManagersStorage::getSubmissionManager(const std::string& problem_id) -> std::shared_ptr<ProblemSubmissionManager> {
    std::lock_guard<std::mutex> lock(mutex_);
    auto it = submission_managers_.find(problem_id);
    if (it != submission_managers_.end()) {
        return it->second;
    }
    auto [iter, inserted] = submission_managers_.emplace(
        problem_id, ProblemSubmissionManagerFactory::instance().create("SendSubmissionToInvoker", problem_id));
    if (!inserted) {
        throw std::runtime_error("Failed to insert new submission manager into storage");
    }
    logger::logDebug("dispatcher", "Created submission manager for problem " + problem_id, 2);
    return iter->second;
}

} // namespace oink_judge::dispatcher
