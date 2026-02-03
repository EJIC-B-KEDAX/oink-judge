#include "services/data_sender/ContentStorage.h"
#include "utils/filesystem.h"
#include "utils/crypto.h"
#include "config/Config.h"
#include "socket/connection_protocol.h"
#include "services/data_sender/ManifestStorage.h"
#include <filesystem>
#include <iostream>

namespace oink_judge::services::data_sender {

using config::Config;
using namespace utils::filesystem;

ContentStorage &ContentStorage::instance() {
    static ContentStorage instance;
    return instance;
}

awaitable<void> ContentStorage::ensure_content_exists(const std::string &content_type, const std::string &content_id) {
    if (!_session) {
        throw std::runtime_error("Session is not initialized.");
    }

    std::cout << "Checking existing " + content_type + " " + content_id << std::endl;

    std::string content_path = Config::config().at("directories").at(content_type + "s").get<std::string>() + "/" + content_id;

    json server_manifest = co_await get_manifest_from_server(content_type, content_id);

    std::cerr << "Received manifest from server: " << server_manifest.dump() << std::endl;

    std::vector<ContentChange> changes = compare_manifests(
        ManifestStorage::instance().get_manifest(content_type, content_id),
        server_manifest);

    if (changes.empty()) {
        co_return;
    }

    for (const auto &change : changes) {
        if (change.type == ContentChange::Type::Added || change.type == ContentChange::Type::Modified) {
            std::string file = co_await get_file_from_server(content_type, content_id, change.file_path);
            store_file(content_path + "/" + change.file_path, file);
        } else if (change.type == ContentChange::Type::Removed) {
            remove_file_or_directory(content_path + "/" + change.file_path);
        }
    }
}

awaitable<void> ContentStorage::update_content_on_server(const std::string &content_type, const std::string &content_id) {
    if (!_session) {
        throw std::runtime_error("Session is not initialized.");
    }

    std::cout << "Updating " + content_type + " " + content_id + " on server" << std::endl;

    std::string content_path = Config::config().at("directories").at(content_type + "s").get<std::string>() + "/" + content_id;
    if (!std::filesystem::exists(content_path)) {
        throw std::runtime_error("Content path does not exist: " + content_path);
    }

    json server_manifest = co_await get_manifest_from_server(content_type, content_id);

    std::vector<ContentChange> changes = compare_manifests(server_manifest,
        ManifestStorage::instance().get_manifest(content_type, content_id).to_json());

    for (const auto &change : changes) {
        if (change.type == ContentChange::Type::Added || change.type == ContentChange::Type::Modified) {
            std::string file_content = load_file(content_path + "/" + change.file_path);
            co_await update_file_on_server(content_type, content_id, change.file_path, file_content);
        } else if (change.type == ContentChange::Type::Removed) {
            co_await remove_file_on_server(content_type, content_id, change.file_path);
        }
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

awaitable<json> ContentStorage::get_manifest_from_server(const std::string &content_type, const std::string &content_id) {
    json manifest = co_await _session->request<json>(
        "{\"request\": \"get_manifest\", \"content_type\": \"" + content_type + "\", \"" + content_type + "_id\": \"" + content_id + "\"}");
    co_return manifest;
}

awaitable<std::string> ContentStorage::get_file_from_server(const std::string &content_type, const std::string &content_id, const std::string &file_path) {
    std::string file_content = co_await _session->request<std::string>(
        "{\"request\": \"get_file\", \"content_type\": \"" + content_type + "\", \"" + content_type + "_id\": \"" + content_id + "\", \"file_path\": \"" + file_path + "\"}");
    co_return file_content;
}

awaitable<void> ContentStorage::update_file_on_server(const std::string &content_type, const std::string &content_id, const std::string &file_path, const std::string &file_content) {
    co_await _session->request<>(
        "{\"request\": \"update_file\", \"content_type\": \"" + content_type + "\", \"" + content_type + "_id\": \"" + content_id + "\", \"file_path\": \"" + file_path + "\", \"file_content\": \"" + utils::crypto::to_base64(file_content) + "\"}");
    co_return;
}

awaitable<void> ContentStorage::remove_file_on_server(const std::string &content_type, const std::string &content_id, const std::string &file_path) {
    co_await _session->request<>(
        "{\"request\": \"remove_file\", \"content_type\": \"" + content_type + "\", \"" + content_type + "_id\": \"" + content_id + "\", \"file_path\": \"" + file_path + "\"}");
    co_return;
}

} // namespace oink_judge::services::data_sender
