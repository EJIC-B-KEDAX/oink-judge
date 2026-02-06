#include "oink_judge/test_node/invoker_protocol.h"

#include "oink_judge/test_node/test_storage.h"

#include <oink_judge/config/config.h>
#include <oink_judge/content_service/content_storage.h>
#include <oink_judge/database/table_submissions.h>
#include <oink_judge/socket/connection_protocol.h>

namespace oink_judge::test_node {

using json = nlohmann::json;
using Config = config::Config;
using TableSubmissions = database::TableSubmissions;

namespace {

[[maybe_unused]] const bool REGISTERED = []() -> bool {
    socket::ProtocolFactory::instance().registerType(
        InvokerProtocol::REGISTERED_NAME,
        [](const std::string& params) -> std::unique_ptr<InvokerProtocol> { return std::make_unique<InvokerProtocol>(); });

    return true;
}();

} // namespace

InvokerProtocol::InvokerProtocol() = default;

awaitable<void> InvokerProtocol::start(std::string start_message) {
    json set_id_request = {{"invoker_id", Config::config()["my_id"].get<std::string>()}};
    co_await sendMessage(set_id_request.dump());
    co_spawn(co_await boost::asio::this_coro::executor, getSession()->receiveMessage(), boost::asio::detached);
}

awaitable<void> InvokerProtocol::receiveMessage(std::string message) {
    json parsed_message = json::parse(message);

    co_spawn(co_await boost::asio::this_coro::executor, getSession()->receiveMessage(), boost::asio::detached);

    if (parsed_message["request"] == "test_submission") {
        std::string submission_id = parsed_message["submission_id"].get<std::string>();
        std::string problem_id = TableSubmissions::instance().problemOfSubmission(submission_id);
        auto test = co_await TestStorage::instance().getTest(problem_id);
        if (!test) {
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
        test->run(submission_id, boxes, parsed_message.value("additional_params", json::object()));
        co_await content_service::ContentStorage::instance().updateContentOnServer("submission", submission_id);
        co_await sendMessage("I am free");
    }
}

void InvokerProtocol::closeSession() {
    std::string host = Config::config().at("hosts").at("dispatcher").get<std::string>();
    short port = Config::config().at("ports").at("dispatcher").get<short>();
    std::string session_type = Config::config().at("sessions").at("dispatcher").get<std::string>();
    co_spawn(getSession()->getExecutor(),
             socket::asyncConnectToTheEndpoint(host, port, session_type,
                                               Config::config().at("start_messages").at("dispatcher").get<std::string>()),
             boost::asio::detached);
}

} // namespace oink_judge::test_node
