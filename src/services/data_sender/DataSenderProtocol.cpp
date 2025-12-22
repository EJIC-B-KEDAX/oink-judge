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

DataSenderProtocol::DataSenderProtocol() : _status(WAIT_REQUEST), _content_type(""), _content_id("") {}

void DataSenderProtocol::start(const std::string &start_message) {
    get_session()->receive_message();
}

void DataSenderProtocol::receive_message(const std::string &message) {
    if (_status == WAIT_REQUEST) {
        json parsed_message = json::parse(message);
        if (parsed_message.at("action") == "get") {
            std::string content_type = parsed_message.at("content_type");
            std::string content_id = parsed_message.at(content_type + "_id");
            uint64_t request_id = parsed_message.at("__id__");

            if (!std::filesystem::exists(Config::config().at("directories").at(content_type + "s_zip").get<std::string>() + "/" + content_id + ".zip")) {
                pack_zip(Config::config().at("directories").at(content_type + "s_zip").get<std::string>() + "/" + content_id + ".zip",
                    Config::config().at("directories").at(content_type + "s").get<std::string>() + "/" + content_id);
            }

            send_message("{\"content_type\": \"" + content_type + "\", \"" + content_type + "_id\": \"" + content_id + "\", \"__id__\": " + std::to_string(request_id) + "}");
            send_message(get_zip(Config::config().at("directories").at(content_type + "s_zip").get<std::string>() + "/" + content_id + ".zip"));
        } else if (parsed_message.at("action") == "update") {
            json header = json::parse(message);

            _content_type = header["content_type"];
            _content_id = header[_content_type + "_id"];
            _status = WAIT_DATA;
        }
    } else if (_status == WAIT_DATA) {
        clear_directory(Config::config().at("directories").at(_content_type + "s").get<std::string>() + "/" + _content_id);
        store_zip(Config::config().at("directories").at(_content_type + "s_zip").get<std::string>() + "/" + _content_id + ".zip", message);
        unpack_zip(Config::config().at("directories").at(_content_type + "s_zip").get<std::string>() + "/" + _content_id + ".zip",
            Config::config().at("directories").at(_content_type + "s").get<std::string>() + "/" + _content_id);

        _status = WAIT_REQUEST;
    }

    get_session()->receive_message();
}

void DataSenderProtocol::close_session() {}

} // namespace oink_judge::services::data_sender
