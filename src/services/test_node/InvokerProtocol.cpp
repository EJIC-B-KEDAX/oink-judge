#include "services/test_node/InvokerProtocol.h"
#include "services/test_node/TestStorage.h"
#include "database/TableSubmissions.h"
#include "services/data_sender/ContentStorage.h"
#include "socket/connection_protocol.h"
#include "config/Config.h"
#include <iostream>

namespace oink_judge::services::test_node {

using json = nlohmann::json;
using Config = config::Config;
using TableSubmissions = database::TableSubmissions;

namespace {

[[maybe_unused]] bool registered = []() -> bool {
    socket::ProtocolFactory::instance().register_type(InvokerProtocol::REGISTERED_NAME,
        [](const std::string &params) -> std::unique_ptr<InvokerProtocol> {
        return std::make_unique<InvokerProtocol>();
    });

    return true;
}();

} // namespace

InvokerProtocol::InvokerProtocol() = default;

void InvokerProtocol::start(const std::string &start_message) {
    std::cout << "Connected to the dispatcher" << std::endl;
    json set_id_request = {
        {"invoker_id", Config::config()["my_id"].get<std::string>()}
    };
    send_message(set_id_request.dump());
    get_session()->receive_message();
}

void InvokerProtocol::receive_message(const std::string &message) {
    json parsed_message = json::parse(message);

    if (parsed_message["request"] == "test_submission") {
        std::string submission_id = parsed_message["submission_id"].get<std::string>();
        std::cout << "Testing submission: " << submission_id << std::endl;
        std::string problem_id = TableSubmissions::instance().problem_of_submission(submission_id);
        std::cout << "Testing problem: " << problem_id << std::endl;
        TestStorage::instance().get_test(problem_id, 
            [session = get_session(), this, submission_id, parsed_message](std::error_code ec, std::shared_ptr<Test> test) {
            if (!test || ec) {

                TableSubmissions::instance().set_score(submission_id, 0.0);
                TableSubmissions::instance().set_verdict_type(submission_id, "FAIL");
                send_message("I am free");
                get_session()->receive_message();

                return;
            }
            data_sender::ContentStorage::instance().ensure_content_exists("submission", submission_id, 
                [session, this, submission_id, parsed_message, test](std::error_code ec) {
                if (ec) {
                    std::cerr << "Error ensuring submission content exists: " << ec.message() << std::endl;

                    TableSubmissions::instance().set_score(submission_id, 0.0);
                    TableSubmissions::instance().set_verdict_type(submission_id, "FAIL");
                    send_message("I am free");
                    get_session()->receive_message();

                    return;
                }
                std::vector<std::string> boxes;
                size_t boxed_required = test->boxes_required();
                for (int i = 0; i < boxed_required; ++i) {
                    boxes.push_back(std::to_string(i));
                }
                test->run(
                    submission_id,
                    boxes,
                    parsed_message.value("additional_params", json::object())
                );
                data_sender::ContentStorage::instance().update_content_on_server("submission", submission_id);
                send_message("I am free");
                get_session()->receive_message();
            });
        });
    }
}

void InvokerProtocol::close_session() {
    std::string host = Config::config().at("hosts").at("dispatcher").get<std::string>();
    short port = Config::config().at("ports").at("dispatcher").get<short>();
    std::string session_type = Config::config().at("sessions").at("dispatcher").get<std::string>();
    socket::async_connect_to_the_endpoint(host, port, session_type,
        Config::config().at("start_messages").at("dispatcher").get<std::string>());
}

} // namespace oink_judge::services::test_node
