#include "services/data_sender/StorageProtocol.h"
#include "config/Config.h"
#include "services/data_sender/zip_utils.h"

namespace oink_judge::services::data_sender {

using json = nlohmann::json;
using Config = config::Config;

namespace {

[[maybe_unused]] bool registered = []() -> bool {
    socket::ProtocolFactory::instance().register_type(StorageProtocol::REGISTERED_NAME,
        [](const std::string &params) -> std::unique_ptr<StorageProtocol> {
        return std::make_unique<StorageProtocol>();
    });

    return true;
}();

} // namespace

StorageProtocol::StorageProtocol() = default;

void StorageProtocol::start(const std::string &start_message) {
    status = WAIT_HEADER;
}

void StorageProtocol::receive_message(const std::string &message) {
    if (status == WAIT_HEADER) {
        json header = json::parse(message);

        content_type = header["content_type"];
        content_id = header[content_type + "_id"];
        status = WAIT_DATA;
        get_session()->receive_message();
    } else if (status == WAIT_DATA) {
        store_zip(Config::config().at("directories").at(content_type + "s_zip").get<std::string>() + "/" + content_id + ".zip", message);
        unpack_zip(Config::config().at("directories").at(content_type + "s_zip").get<std::string>() + "/" + content_id + ".zip",
            Config::config().at("directories").at(content_type + "s").get<std::string>() + "/" + content_id);
            
        status = WAIT_HEADER;   
    }
}

void StorageProtocol::close_session() {
    // reconnect to the server
}

void StorageProtocol::set_session(std::weak_ptr<socket::Session> session) {
    _session = session;
}

std::shared_ptr<socket::Session> StorageProtocol::get_session() const {
    return _session.lock();
}

void StorageProtocol::request_internal(const std::string &message, const callback_t &callback) {
    // Not implemented
}

} // namespace oink_judge::services::data_sender
