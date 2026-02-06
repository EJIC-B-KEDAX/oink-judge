#include "oink_judge/dispatcher/send_submission_to_invoker.h"

#include "oink_judge/dispatcher/testing_queue.h"

namespace oink_judge::dispatcher {

namespace {

[[maybe_unused]] const bool REGISTERED = []() -> bool {
    ProblemSubmissionManagerFactory::instance().registerType(
        SendSubmissionToInvoker::REGISTERED_NAME,
        [](const std::string& params, const std::string& problem_id) -> std::shared_ptr<ProblemSubmissionManager> {
            return std::make_shared<SendSubmissionToInvoker>(problem_id);
        });
    return true;
}();

} // namespace

SendSubmissionToInvoker::SendSubmissionToInvoker(std::string problem_id) : problem_id_(std::move(problem_id)) {}

auto SendSubmissionToInvoker::handleSubmission(const std::string& submission_id) -> void {
    TestingQueue::instance().pushSubmission(submission_id);
}

} // namespace oink_judge::dispatcher
