#include "services/test_node/DefaultInvokerSessionEventHandler.h"
#include "services/test_node/TestStorage.h"
#include "services/test_node/TableSubmissions.h"
#include "services/data_sender/ContentStorage.h"
#include "config/Config.h"
#include <iostream>

namespace oink_judge::services::test_node {

using json = nlohmann::json;

namespace {

[[maybe_unused]] bool registered = []() -> bool {
    socket::BasicSessionEventHandlerFactory::instance().register_type(DefaultInvokerSessionEventHandler::REGISTERED_NAME,
        [](const std::string &params) -> std::unique_ptr<DefaultInvokerSessionEventHandler> {
        return std::make_unique<DefaultInvokerSessionEventHandler>();
    });

    return true;
}();

} // namespace

DefaultInvokerSessionEventHandler::DefaultInvokerSessionEventHandler() = default;

void DefaultInvokerSessionEventHandler::start(const std::string &start_message) {
    std::cout << "Connected to the dispatcher" << std::endl;
    json set_id_request = {
        {"invoker_id", config::Config::config()["my_id"].get<std::string>()}
    };
    std::cout << "Sending invoker ID: " << set_id_request.dump() << std::endl;
    get_session().lock()->send_message(set_id_request.dump());
    get_session().lock()->receive_message();
}

void DefaultInvokerSessionEventHandler::receive_message(const std::string &message) {
    json parsed_message = json::parse(message);

    if (parsed_message["request"] == "test_submission") {
        std::cout << "Testing submission: " << parsed_message["submission_id"].get<std::string>() << std::endl;
        std::string problem_id = TableSubmissions::instance().problem_of_submission(parsed_message["submission_id"].get<std::string>());
        std::cout << "Testing problem: " << problem_id << std::endl;
        std::shared_ptr<Test> test = TestStorage::instance().get_test(problem_id);
        data_sender::ContentStorage::instance().ensure_content_exists("submission", parsed_message["submission_id"].get<std::string>());
        std::vector<std::string> boxes;
        size_t boxed_required = test->boxes_required();
        for (int i = 0; i < boxed_required; ++i) {
            boxes.push_back(std::to_string(i));
        }
        test->run(
            parsed_message["submission_id"].get<std::string>(),
            boxes,
            parsed_message.value("additional_params", json::object())
        );
    }

    get_session().lock()->send_message("I am free");
    get_session().lock()->receive_message();
}

void DefaultInvokerSessionEventHandler::close_session() {
    // reconnect to the server
}

void DefaultInvokerSessionEventHandler::set_session(std::weak_ptr<socket::Session> session) {
    _session = session;
}

std::weak_ptr<socket::Session> DefaultInvokerSessionEventHandler::get_session() const {
    return _session;
}

} // namespace oink_judge::services::test_node
