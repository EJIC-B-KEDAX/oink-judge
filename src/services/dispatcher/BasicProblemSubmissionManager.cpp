#include "services/dispatcher/BasicProblemSubmissionManager.h"
#include "services/dispatcher/TestingQueue.h"

namespace oink_judge::services::dispatcher {

namespace {

[[maybe_unused]] bool registerd = []() -> bool {
    ProblemSubmissionManagerFactory::instance().register_type(
        BasicProblemSubmissionManager::REGISTERED_NAME,
        [](const std::string &params, const std::string &problem_id) -> std::shared_ptr<ProblemSubmissionManager> {
            return std::make_shared<BasicProblemSubmissionManager>(problem_id);
        }
    );
    return true;
}();

} // namespace

BasicProblemSubmissionManager::BasicProblemSubmissionManager(const std::string &problem_id)
    : _problem_id(problem_id) {}

void BasicProblemSubmissionManager::handle_submission(const std::string &submission_id) {
    TestingQueue::instance().push_submission(submission_id);
}

} // namespace oink_judge::services::dispatcher
