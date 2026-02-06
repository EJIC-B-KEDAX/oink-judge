#include "oink_judge/dispatcher/protocol_with_fastapi.h"

#include <oink_judge/content_service/content_storage.h>
#include <oink_judge/database/table_submissions.h>

namespace oink_judge::dispatcher {

using ContentStorage = content_service::ContentStorage;
using TableSubmissions = database::TableSubmissions;

namespace {

[[maybe_unused]] const bool REGISTERED = []() -> bool {
    socket::ProtocolFactory::instance().registerType(
        ProtocolWithFastAPI::REGISTERED_NAME,
        [](const std::string& params) -> std::unique_ptr<socket::Protocol> { return std::make_unique<ProtocolWithFastAPI>(); });
    return true;
}();

} // namespace

ProtocolWithFastAPI::ProtocolWithFastAPI() = default;

auto ProtocolWithFastAPI::start(std::string start_message) -> awaitable<void> {
    co_spawn(co_await boost::asio::this_coro::executor, getSession()->receiveMessage(), boost::asio::detached);
}

auto ProtocolWithFastAPI::receiveMessage(std::string message) -> awaitable<void> {
    auto json_message = nlohmann::json::parse(message);
    std::string request_type = json_message["request"];

    if (request_type == "handle_submission") {
        std::string submission_id = json_message["submission_id"];
        std::string problem_id = TableSubmissions::instance().problemOfSubmission(submission_id);
        std::string request_id = json_message["__id__"];

        if (!submission_managers_.contains(problem_id)) {
            // ContentStorage::instance().ensure_content_exists("problem", problem_id, [](std::error_code ec) {
            //     if (ec) {
            //         throw std::runtime_error("Error ensuring problem content exists: " + ec.message());
            //     }
            // });

            submission_managers_[problem_id] =
                ProblemSubmissionManagerFactory::instance().create("BasicProblemSubmissionManager", problem_id);
            // TODO: Get submission manager type from problem config

            if (!submission_managers_[problem_id]) {
                throw std::runtime_error("Failed to create ProblemSubmissionManager for problem: " + problem_id);
            }
        }

        submission_managers_[problem_id]->handleSubmission(submission_id);

        nlohmann::json response = {{"__id__", request_id}, {"status", "success"}};
        co_await getSession()->sendMessage(response.dump());
    }

    co_spawn(co_await boost::asio::this_coro::executor, getSession()->receiveMessage(), boost::asio::detached);
}

auto ProtocolWithFastAPI::closeSession() -> void {}

} // namespace oink_judge::dispatcher
