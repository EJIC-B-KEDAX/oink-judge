#include "services/dispatcher/BasicProblemSubmissionManager.h"

#include "services/dispatcher/TestingQueue.h"

namespace oink_judge::services::dispatcher {

namespace {

[[maybe_unused]] const bool REGISTERED = []() -> bool {
    ProblemSubmissionManagerFactory::instance().register_type(
        BasicProblemSubmissionManager::REGISTERED_NAME,
        [](const std::string& params, const std::string& problem_id) -> std::shared_ptr<ProblemSubmissionManager> {
            return std::make_shared<BasicProblemSubmissionManager>(problem_id);
        });
    return true;
}();

} // namespace

BasicProblemSubmissionManager::BasicProblemSubmissionManager(std::string problem_id) : problem_id_(std::move(problem_id)) {}

auto BasicProblemSubmissionManager::handleSubmission(const std::string& submission_id) -> void {
    TestingQueue::instance().push_submission(submission_id);
}

} // namespace oink_judge::services::dispatcher
