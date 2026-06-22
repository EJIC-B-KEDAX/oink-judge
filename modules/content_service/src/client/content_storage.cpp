#include "oink_judge/content_service/client/content_storage.h"

#include "oink_judge/content_service/client/config_utils.h"
#include "oink_judge/content_service/client/content_service_stub.h"
#include "oink_judge/content_service/config_utils.h"
#include "oink_judge/content_service/manifest_storage.h"

#include <oink_judge/config/common_utils.h>
#include <oink_judge/logger/logger.h>
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
    fs::path content_path = requireHasValue(getContentDirectory(content_type)) / content_id;

    json server_manifest = co_await getManifestFromServer(content_type, content_id);

    std::vector<ContentChange> changes =
        compareManifests(ManifestStorage::instance().getManifest(content_type, content_id), server_manifest);

    if (changes.empty()) {
        co_return;
    }

    logger::logInfo("content_storage", "Syncing " + std::to_string(changes.size()) + " change(s) for " + content_type + "/" +
                                        content_id);

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
    fs::path content_path = requireHasValue(getContentDirectory(content_type)) / content_id;
    if (!std::filesystem::exists(content_path)) {
        throw std::runtime_error("Content path does not exist: " + content_path.string());
    }

    json server_manifest = co_await getManifestFromServer(content_type, content_id);

    std::vector<ContentChange> changes =
        compareManifests(server_manifest, ManifestStorage::instance().getManifest(content_type, content_id).toJson());

    if (changes.empty()) {
        co_return;
    }

    logger::logInfo("content_storage", "Uploading " + std::to_string(changes.size()) + " change(s) for " + content_type + "/" +
                                        content_id);

    for (const auto& change : changes) {
        if (change.type == ContentChange::Type::ADDED) {
            std::string file_content = loadFile(content_path / change.file_path);
            co_await createFileOnServer(content_type, content_id, change.file_path, file_content);
        } else if (change.type == ContentChange::Type::MODIFIED) {
            std::string file_content = loadFile(content_path / change.file_path);
            co_await updateFileOnServer(content_type, content_id, change.file_path, file_content);
        } else if (change.type == ContentChange::Type::REMOVED) {
            co_await removeFileOnServer(content_type, content_id, change.file_path);
        }
    }
}

ContentStorage::ContentStorage() {
    auto stub_opt = getContentStorageStub();
    if (!stub_opt.has_value()) {
        throw std::runtime_error("Failed to create ContentServiceStub: optional expected with value");
    }
    stub_ = std::move(*stub_opt);
}

ContentStorage::ContentStorage(std::unique_ptr<ContentServiceStub> stub) : stub_(std::move(stub)) {}

auto ContentStorage::getManifestFromServer(std::string content_type, std::string content_id) -> awaitable<json> { // NOLINT
    auto manifest_exp = co_await stub_->getManifest(content_type, content_id);
    if (!manifest_exp.has_value()) {
        throw std::runtime_error("Failed to get manifest from server: " + manifest_exp.error().error_message());
    }

    co_return *manifest_exp;
}

auto ContentStorage::getFileFromServer(std::string content_type, std::string content_id, std::string file_path) // NOLINT
    -> awaitable<std::string> {
    auto file_exp = co_await stub_->getFile(content_type, content_id, file_path);
    if (!file_exp.has_value()) {
        throw std::runtime_error("Failed to get file from server: " + file_exp.error().error_message());
    }
    co_return *file_exp;
}

auto ContentStorage::createFileOnServer(std::string content_type, std::string content_id, std::string file_path, // NOLINT
                                        std::string file_content) -> awaitable<void> {
    auto update_exp = co_await stub_->createFile(content_type, content_id, file_path, file_content);
    if (!update_exp.has_value()) {
        throw std::runtime_error("Failed to create file on server: " + update_exp.error().error_message());
    }
}

auto ContentStorage::updateFileOnServer(std::string content_type, std::string content_id, std::string file_path, // NOLINT
                                        std::string file_content) -> awaitable<void> {
    auto update_exp = co_await stub_->updateFile(content_type, content_id, file_path, file_content);
    if (!update_exp.has_value()) {
        throw std::runtime_error("Failed to update file on server: " + update_exp.error().error_message());
    }
}

auto ContentStorage::removeFileOnServer(std::string content_type, std::string content_id, std::string file_path) // NOLINT
    -> awaitable<void> {
    auto delete_exp = co_await stub_->deleteFile(content_type, content_id, file_path);
    if (!delete_exp.has_value()) {
        throw std::runtime_error("Failed to delete file on server: " + delete_exp.error().error_message());
    }
}

} // namespace oink_judge::content_service
