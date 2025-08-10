#include "services/data_sender/StorageSessionEventHandler.h"
#include "config/Config.h"
#include "services/data_sender/zip_utils.h"

namespace oink_judge::services::data_sender {

using json = nlohmann::json;
using Config = config::Config;

namespace {

[[maybe_unused]] bool registered = []() -> bool {
    socket::BasicSessionEventHandlerFactory::instance().register_type(StorageSessionEventHandler::REGISTERED_NAME,
        [](const std::string &params) -> std::unique_ptr<StorageSessionEventHandler> {
        return std::make_unique<StorageSessionEventHandler>();
    });

    return true;
}();

} // namespace

StorageSessionEventHandler::StorageSessionEventHandler() = default;

void StorageSessionEventHandler::start(const std::string &start_message) {
    status = WAIT_HEADER;
}

void StorageSessionEventHandler::receive_message(const std::string &message) {
    if (status == WAIT_HEADER) {
        json header = json::parse(message);

        content_type = header["content_type"];
        content_id = header[content_type + "_id"];
        status = WAIT_DATA;
    } else if (status == WAIT_DATA) {
        store_zip(Config::config().at("directories").at(content_type + "s_zip").get<std::string>() + "/" + content_id + ".zip", message);
        unpack_zip(Config::config().at("directories").at(content_type + "s_zip").get<std::string>() + "/" + content_id + ".zip",
            Config::config().at("directories").at(content_type + "s").get<std::string>() + "/" + content_id);
            
        status = WAIT_HEADER;   
    }
}

void StorageSessionEventHandler::close_session() {
    // reconnect to the server
}

void StorageSessionEventHandler::set_session(std::weak_ptr<socket::Session> session) {
    _session = session;
}

std::weak_ptr<socket::Session> StorageSessionEventHandler::get_session() const {
    return _session;
}

} // namespace oink_judge::services::data_sender
