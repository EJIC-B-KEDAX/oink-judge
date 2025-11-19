#include "services/data_sender/DataSenderProtocol.h"
#include "config/Config.h"
#include "services/data_sender/zip_utils.h"

namespace oink_judge::services::data_sender {

using json = nlohmann::json;
using Config = config::Config;

namespace {

[[maybe_unused]] bool registered = []() -> bool {
    socket::ProtocolFactory::instance().register_type(DataSenderProtocol::REGISTERED_NAME,
        [](const std::string &params) -> std::unique_ptr<DataSenderProtocol> {
        return std::make_unique<DataSenderProtocol>();
    });

    return true;
}();

} // namespace

DataSenderProtocol::DataSenderProtocol() = default;

void DataSenderProtocol::start(const std::string &start_message) {
    get_session()->receive_message();
}

void DataSenderProtocol::receive_message(const std::string &message) {
    json parsed_message = json::parse(message);
    if (parsed_message.at("action") == "get") {
        std::string content_type = parsed_message.at("content_type");
        std::string content_id = parsed_message.at(content_type + "_id");

        if (!std::filesystem::exists(Config::config().at("directories").at(content_type + "s_zip").get<std::string>() + "/" + content_id + ".zip")) {
            pack_zip(Config::config().at("directories").at(content_type + "s_zip").get<std::string>() + "/" + content_id + ".zip",
                Config::config().at("directories").at(content_type + "s").get<std::string>() + "/" + content_id);
        }

        get_session()->send_message(
            "{\"content_type\": \"" + content_type + "\", \"" + content_type + "_id\": \"" + content_id + "\"}");
        get_session()->send_message(get_zip(Config::config().at("directories").at(content_type + "s_zip").get<std::string>() + "/" + content_id + ".zip"));
    }

    get_session()->receive_message();
}

void DataSenderProtocol::close_session() {}

void DataSenderProtocol::set_session(std::weak_ptr<socket::Session> session) {
    _session = session;
}

std::shared_ptr<socket::Session> DataSenderProtocol::get_session() const {
    return _session.lock();
}

void DataSenderProtocol::request_internal(const std::string &message, const callback_t &callback) {
    throw std::runtime_error("Request not supported for DataSenderProtocol");
}

} // namespace oink_judge::services::data_sender
