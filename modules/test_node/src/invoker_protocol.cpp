#include "oink_judge/test_node/invoker_protocol.h"

#include "oink_judge/test_node/config_utils.h"
#include "oink_judge/test_node/test_storage.h"

#include <oink_judge/config/common_utils.h>
#include <oink_judge/content_service/client/content_storage.h>
#include <oink_judge/database/table_submissions.h>
#include <oink_judge/logger/logger.h>
#include <oink_judge/socket/client_config_utils.h>
#include <oink_judge/socket/connection_protocol.h>

namespace oink_judge::test_node {

using config::requireHasValue;
using database::TableSubmissions;
using nlohmann::json;

InvokerProtocol::InvokerProtocol() = default;

auto InvokerProtocol::start(std::string start_message) -> awaitable<void> {
    json set_id_request = {{"invoker_id", requireHasValue(getMyTestNodeId())}};
    co_await sendMessage(set_id_request.dump());
    co_spawn(co_await boost::asio::this_coro::executor, getSession()->receiveMessage(), boost::asio::detached);
}

auto InvokerProtocol::receiveMessage(std::string message) -> awaitable<void> {
    json parsed_message = json::parse(message);

    co_spawn(co_await boost::asio::this_coro::executor, getSession()->receiveMessage(), boost::asio::detached);

    if (parsed_message["request"] == "test_submission") {
        std::string submission_id = parsed_message["submission_id"].get<std::string>();
        logger::logInfo("InvokerProtocol", "Received test request for submission_id: " + submission_id);
        std::string problem_id = TableSubmissions::instance().problemOfSubmission(submission_id);
        auto test = co_await TestStorage::instance().getTest(problem_id);
        if (!test) {
            logger::logError("InvokerProtocol", "Test not found for problem_id: " + problem_id);
            TableSubmissions::instance().setScore(submission_id, 0.0);
            TableSubmissions::instance().setVerdictType(submission_id, "FAIL");
            co_await sendMessage("I am free");
            co_return;
        }
        // TODO use try catch here
        co_await content_service::ContentStorage::instance().ensureContentExists("submission", submission_id);
        // if (ec) {
        //     std::cerr << "Error ensuring submission content exists: " << ec.message() << std::endl;

        //     TableSubmissions::instance().setScore(submission_id, 0.0);
        //     TableSubmissions::instance().setVerdictType(submission_id, "FAIL");
        //     sendMessage("I am free");
        //     getSession()->receiveMessage();

        //     return;
        // }
        std::vector<std::string> boxes;
        size_t boxes_required = test->boxesRequired();
        boxes.reserve(boxes_required);
        for (int i = 0; i < boxes_required; ++i) {
            boxes.push_back(std::to_string(i));
        }
        logger::logInfo("InvokerProtocol", "Starting test for submission_id: " + submission_id);
        test->run(submission_id, boxes, parsed_message.value("additional_params", json::object()));
        co_await content_service::ContentStorage::instance().updateContentOnServer("submission", submission_id);
        co_await sendMessage("I am free");
    }
}

auto InvokerProtocol::closeSession() -> void {
    socket::ConnectionConfig connection_config = requireHasValue(socket::getConnectionConfig("dispatcher"));
    co_spawn(getSession()->getExecutor(),
             socket::asyncConnectToTheEndpoint(connection_config.host, connection_config.port, connection_config.session_type,
                                               connection_config.start_message),
             boost::asio::detached);
}

auto registerInvokerProtocolType() -> void {

    socket::ProtocolFactory::instance().registerType(
        InvokerProtocol::REGISTERED_NAME,
        [](const std::string& params, const boost::asio::any_io_executor& executor) -> std::unique_ptr<InvokerProtocol> {
            return std::make_unique<InvokerProtocol>();
        });
}

} // namespace oink_judge::test_node
