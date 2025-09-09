#include "services/data_sender/ContentStorage.h"
#include "services/data_sender/zip_utils.h"
#include "config/Config.h"
#include "socket/connection_protocol.h"
#include <filesystem>

namespace oink_judge::services::data_sender {

using Config = config::Config;

ContentStorage &ContentStorage::instance() {
    static ContentStorage instance;
    return instance;
}

void ContentStorage::ensure_content_exists(const std::string &content_type, const std::string &content_id) {
    if (!_session) {
        throw std::runtime_error("Session is not initialized.");
    }

    if (std::filesystem::exists(Config::config().at("directories").at(content_type + "s").get<std::string>() + "/" + content_id)) {
        return;
    }

    if (std::filesystem::exists(Config::config().at("directories").at(content_type + "s_zip").get<std::string>() + "/" + content_id + ".zip")) {
        unpack_zip(Config::config().at("directories").at(content_type + "s_zip").get<std::string>() + "/" + content_id + ".zip",
            Config::config().at("directories").at(content_type + "s").get<std::string>() + "/" + content_id);
        return;
    }

    // TODO check content version (configurable)
    // TODO make configurable directories related to content_type

    _session->send_message("{\"action\": \"get\", \"content_type\": \"" + content_type + "\", \"" + content_type + "_id\": \"" + content_id + "\"}");
    _session->receive_message();
}

ContentStorage::ContentStorage() {
    _session = socket::connect_to_the_endpoint(
        Config::config().at("hosts").at("data_sender").get<std::string>(),
        Config::config().at("ports").at("data_sender").get<short>(),
        Config::config().at("sessions").at("data_sender").get<std::string>(),
        Config::config().at("start_messages").at("data_sender").get<std::string>());

    if (!_session) {
        throw std::runtime_error("Failed to connect to the data sender endpoint.");
    }
}

} // namespace oink_judge::services::data_sender
