#include "oink_judge/content_service/client/content_storage.h"

#include "oink_judge/content_service/config_utils.h"
#include "oink_judge/content_service/manifest_storage.h"

#include <oink_judge/config/common_utils.h>
#include <oink_judge/logger/logger.h>
#include <oink_judge/socket/client_config_utils.h>
#include <oink_judge/socket/connection_protocol.h>
#include <oink_judge/utils/crypto.h>
#include <oink_judge/utils/filesystem.h>

#include <filesystem>

namespace oink_judge::content_service {

using namespace utils::filesystem;
namespace fs = std::filesystem;

using config::requireHasValue;

auto ContentStorage::instance() -> ContentStorage& {
    static ContentStorage instance;
    return instance;
}

auto ContentStorage::ensureContentExists(std::string content_type, std::string content_id) -> awaitable<void> {
    if (!session_) {
        throw std::runtime_error("Session is not initialized.");
    }

    fs::path content_path = requireHasValue(getContentDirectory(content_type)) / content_id;

    json server_manifest = co_await getManifestFromServer(content_type, content_id);

    std::vector<ContentChange> changes =
        compareManifests(ManifestStorage::instance().getManifest(content_type, content_id), server_manifest);

    if (changes.empty()) {
        co_return;
    }

    for (const auto& change : changes) {
        if (change.type == ContentChange::Type::ADDED || change.type == ContentChange::Type::MODIFIED) {
            std::string file = co_await getFileFromServer(content_type, content_id, change.file_path);
            storeFile(content_path / change.file_path, file);
            setPermissions(content_path / change.file_path, getPermissionsFromManifest(server_manifest, change.file_path));
        } else if (change.type == ContentChange::Type::ATTRIBUTES_CHANGED) {
            setPermissions(content_path / change.file_path, getPermissionsFromManifest(server_manifest, change.file_path));
        } else if (change.type == ContentChange::Type::REMOVED) {
            removeFileOrDirectory(content_path / change.file_path);
        }
    }
}

auto ContentStorage::updateContentOnServer(std::string content_type, std::string content_id) -> awaitable<void> {
    if (!session_) {
        throw std::runtime_error("Session is not initialized.");
    }

    fs::path content_path = requireHasValue(getContentDirectory(content_type)) / content_id;
    if (!std::filesystem::exists(content_path)) {
        throw std::runtime_error("Content path does not exist: " + content_path.string());
    }

    json server_manifest = co_await getManifestFromServer(content_type, content_id);

    std::vector<ContentChange> changes =
        compareManifests(server_manifest, ManifestStorage::instance().getManifest(content_type, content_id).toJson());

    for (const auto& change : changes) {
        if (change.type == ContentChange::Type::ADDED || change.type == ContentChange::Type::MODIFIED) {
            std::string file_content = loadFile(content_path / change.file_path);
            co_await updateFileOnServer(content_type, content_id, change.file_path, file_content);
        } else if (change.type == ContentChange::Type::REMOVED) {
            co_await removeFileOnServer(content_type, content_id, change.file_path);
        }
    }
}

ContentStorage::ContentStorage() {
    // auto connection_config = requireHasValue(socket::getConnectionConfig("content_service"));
    // session_ = socket::connectToTheEndpoint(connection_config.host, connection_config.port, connection_config.session_type,
    //                                         connection_config.start_message);

    if (!session_) {
        throw std::runtime_error("Failed to connect to the data sender endpoint.");
    }
}

auto ContentStorage::ensureConnection() -> awaitable<void> {
    if (!session_) {
        auto connection_config = requireHasValue(socket::getConnectionConfig("content_service"));
        session_ = co_await socket::asyncConnectToTheEndpoint(connection_config.host, connection_config.port,
                                                              connection_config.session_type, connection_config.start_message);
    }
}

auto ContentStorage::getManifestFromServer(std::string content_type, std::string content_id) -> awaitable<json> {
    json manifest = co_await session_->request<json>(R"({"request": "get_manifest", "content_type": ")" + content_type +
                                                     "\", \"" + content_type + "_id\": \"" + content_id + "\"}");
    co_return manifest;
}

auto ContentStorage::getFileFromServer(std::string content_type, std::string content_id, std::string file_path)
    -> awaitable<std::string> {
    std::string file_content = co_await session_->request<std::string>(R"({"request": "get_file", "content_type": ")" +
                                                                       content_type + "\", \"" + content_type + "_id\": \"" +
                                                                       content_id + R"(", "file_path": ")" + file_path + "\"}");
    co_return file_content;
}

auto ContentStorage::updateFileOnServer(std::string content_type, std::string content_id, std::string file_path,
                                        std::string file_content) -> awaitable<void> {
    co_await session_->request<>(R"({"request": "update_file", "content_type": ")" + content_type + "\", \"" + content_type +
                                 "_id\": \"" + content_id + R"(", "file_path": ")" + file_path + R"(", "file_content": ")" +
                                 utils::crypto::toBase64(file_content) + "\"}");
    co_return;
}

auto ContentStorage::removeFileOnServer(std::string content_type, std::string content_id, std::string file_path)
    -> awaitable<void> {
    co_await session_->request<>(R"({"request": "remove_file", "content_type": ")" + content_type + "\", \"" + content_type +
                                 "_id\": \"" + content_id + R"(", "file_path": ")" + file_path + "\"}");
    co_return;
}

} // namespace oink_judge::content_service
