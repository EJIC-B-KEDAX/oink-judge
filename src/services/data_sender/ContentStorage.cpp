#include "services/data_sender/ContentStorage.h"
#include "services/data_sender/zip_utils.h"
#include "config/Config.h"
#include "socket/connection_protocol.h"
#include <filesystem>
#include <iostream>

namespace oink_judge::services::data_sender {

using Config = config::Config;

ContentStorage &ContentStorage::instance() {
    static ContentStorage instance;
    return instance;
}

void ContentStorage::ensure_content_exists(const std::string &content_type, const std::string &content_id, CallbackFunc callback) {
    if (!_session) {
        throw std::runtime_error("Session is not initialized.");
    }

    std::cout << "Checking existing " + content_type + " " + content_id << std::endl;

    if (std::filesystem::exists(Config::config().at("directories").at(content_type + "s").get<std::string>() + "/" + content_id)) {
        callback(std::error_code{});
        return;
    }

    if (std::filesystem::exists(Config::config().at("directories").at(content_type + "s_zip").get<std::string>() + "/" + content_id + ".zip")) {
        unpack_zip(Config::config().at("directories").at(content_type + "s_zip").get<std::string>() + "/" + content_id + ".zip",
            Config::config().at("directories").at(content_type + "s").get<std::string>() + "/" + content_id);
        callback(std::error_code{});
        return;
    }

    // TODO check content version (configurable)
    // TODO make configurable directories related to content_type

    _session->request("{\"action\": \"get\", \"content_type\": \"" + content_type + "\", \"" + content_type + "_id\": \"" + content_id + "\"}",
        callback);
}

void ContentStorage::update_content_on_server(const std::string &content_type, const std::string &content_id) {
    if (!_session) {
        throw std::runtime_error("Session is not initialized.");
    }

    std::cout << "Updating " + content_type + " " + content_id + " on server" << std::endl;

    nlohmann::json request_json;
    request_json["action"] = "update";
    request_json["content_type"] = content_type;
    request_json[content_type + "_id"] = content_id;

    std::string content_path = Config::config().at("directories").at(content_type + "s").get<std::string>() + "/" + content_id;
    if (!std::filesystem::exists(content_path)) {
        throw std::runtime_error("Content path does not exist: " + content_path);
    }

    try {
        std::string zip_content;
        std::string zip_path = Config::config().at("directories").at(content_type + "s_zip").get<std::string>() + "/" + content_id + ".zip";

        remove_file_or_directory(zip_path);
        pack_zip(zip_path, content_path);
        zip_content = get_zip(zip_path);

        _session->send_message(request_json.dump());
        _session->send_message(zip_content);
    } catch (const std::exception &e) {
        throw std::runtime_error("Failed to pack or send content: " + std::string(e.what()));
    }
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
