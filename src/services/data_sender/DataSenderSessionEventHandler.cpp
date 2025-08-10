#include "services/data_sender/DataSenderSessionEventHandler.h"
#include "config/Config.h"
#include "services/data_sender/zip_utils.h"

namespace oink_judge::services::data_sender {

using json = nlohmann::json;
using Config = config::Config;

namespace {

[[maybe_unused]] bool registered = []() -> bool {
    socket::BasicSessionEventHandlerFactory::instance().register_type(DataSenderSessionEventHandler::REGISTERED_NAME,
        [](const std::string &params) -> std::unique_ptr<DataSenderSessionEventHandler> {
        return std::make_unique<DataSenderSessionEventHandler>();
    });

    return true;
}();

} // namespace

DataSenderSessionEventHandler::DataSenderSessionEventHandler() = default;

void DataSenderSessionEventHandler::start(const std::string &start_message) {
    _session.lock()->receive_message();
}

void DataSenderSessionEventHandler::receive_message(const std::string &message) {
    json parsed_message = json::parse(message);
    if (parsed_message.at("action") == "get") {
        std::string content_type = parsed_message.at("content_type");
        std::string content_id = parsed_message.at(content_type + "_id");

        if (!std::filesystem::exists(Config::config().at("directories").at(content_type + "s_zip").get<std::string>() + "/" + content_id + ".zip")) {
            pack_zip(Config::config().at("directories").at(content_type + "s_zip").get<std::string>() + "/" + content_id + ".zip",
                Config::config().at("directories").at(content_type + "s").get<std::string>() + "/" + content_id);
        }

        _session.lock()->send_message(
            "{\"content_type\": \"" + content_type + "\", \"" + content_type + "_id\": \"" + content_id + "\"}");
        _session.lock()->send_message(get_zip(Config::config().at("directories").at(content_type + "s_zip").get<std::string>() + "/" + content_id + ".zip"));
    }

    _session.lock()->receive_message();
}

void DataSenderSessionEventHandler::close_session() {}

void DataSenderSessionEventHandler::set_session(std::weak_ptr<socket::Session> session) {
    _session = session;
}

std::weak_ptr<socket::Session> DataSenderSessionEventHandler::get_session() const {
    return _session;
}

} // namespace oink_judge::services::data_sender
