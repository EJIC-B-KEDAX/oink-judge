#include "services/dispatcher/SessionWithFastAPIEventHandler.h"
#include "services/data_sender/ContentStorage.h"

namespace oink_judge::services::dispatcher {

using ContentStorage = data_sender::ContentStorage;

namespace {

[[maybe_unused]] bool registered = []() -> bool {
    socket::BasicSessionEventHandlerFactory::instance().register_type(
        SessionWithFastAPIEventHandler::REGISTERED_NAME,
        [](const std::string &params) -> std::unique_ptr<socket::SessionEventHandler> {
            return std::make_unique<SessionWithFastAPIEventHandler>();
        }
    );
    return true;
}();

} // namespace

SessionWithFastAPIEventHandler::SessionWithFastAPIEventHandler() = default;

void SessionWithFastAPIEventHandler::start(const std::string &start_message) {
    get_session().lock()->receive_message();
}

void SessionWithFastAPIEventHandler::receive_message(const std::string &message) {
    auto json_message = nlohmann::json::parse(message);
    std::string request_type = json_message["request_type"];

    if (request_type == "handle_submission") {
        std::string submission_id = json_message["submission_id"];
        // std::string problem_id = json_message["problem_id"]; // TODO: Get it from database
        std::string problem_id = "4"; // Temporary hardcoded value

        if (_submission_managers.find(problem_id) == _submission_managers.end()) {
            ContentStorage::instance().ensure_content_exists("problem", problem_id);
            _submission_managers[problem_id] = ProblemSubmissionManagerFactory::instance().create("BasicProblemSubmissionManager", problem_id);
            // TODO: Get submission manager type from problem config

            if (!_submission_managers[problem_id]) {
                throw std::runtime_error("Failed to create ProblemSubmissionManager for problem: " + problem_id);
            }
        }

        _submission_managers[problem_id]->handle_submission(submission_id);
    }

    get_session().lock()->receive_message();
}

void SessionWithFastAPIEventHandler::close_session() {}

void SessionWithFastAPIEventHandler::set_session(std::weak_ptr<socket::Session> session) {
    _session = session;
}

std::weak_ptr<socket::Session> SessionWithFastAPIEventHandler::get_session() const {
    return _session;
}

} // namespace oink_judge::services::dispatcher
