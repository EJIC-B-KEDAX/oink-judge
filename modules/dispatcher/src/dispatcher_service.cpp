#include "oink_judge/dispatcher/dispatcher_service.h"

#include "oink_judge/dispatcher/submission_managers_storage.h"

#include <oink_judge/content_service/client/content_storage.h>
#include <oink_judge/database/async_table_submissions.h>
#include <oink_judge/logger/logger.h>

namespace oink_judge::dispatcher {

using AsyncTableSubmissions = database::AsyncTableSubmissions;

auto handleSubmissionHandler(HandleSubmissionRPC& rpc, HandleSubmissionRequest& request) -> awaitable<void> { // NOLINT
    std::string error_message;
    try {
        const std::string& submission_id = request.submission_id();
        std::string problem_id = co_await AsyncTableSubmissions::instance().problemOfSubmission(submission_id);

        logger::logInfo("dispatcher_service", "Received handle_submission for submission " + submission_id);

        co_await content_service::ContentStorage::instance().ensureContentExists("problem", problem_id);

        auto submission_manager = SubmissionManagersStorage::instance().getSubmissionManager(problem_id);

        submission_manager->handleSubmission(submission_id);

        logger::logInfo("dispatcher_service", "Handled submission " + submission_id + " for problem " + problem_id);

        co_await rpc.finish(google::protobuf::Empty{}, grpc::Status::OK);
        co_return;
    } catch (const std::exception& e) {
        error_message = e.what();
        logger::logError("dispatcher_service", "Failed to handle submission " + request.submission_id() + ": " + error_message);
    }
    co_await rpc.finish_with_error(grpc::Status(grpc::StatusCode::INTERNAL, error_message));
}

} // namespace oink_judge::dispatcher
