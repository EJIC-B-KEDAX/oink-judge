#include "oink_judge/content_service/content_manifest.h"

#include "oink_judge/content_service/config_utils.h"

#include <oink_judge/config/common_utils.h>
#include <oink_judge/logger/logger.h>
#include <oink_judge/utils/crypto.h>
#include <oink_judge/utils/filesystem.h>

#include <filesystem>

namespace oink_judge::content_service {

using config::requireHasValue;

namespace {

auto getPathToContentDirectory(const std::string& content_type, const std::string& content_id) -> fs::path {
    fs::path path_to_content = requireHasValue(getContentDirectory(content_type));
    return path_to_content / content_id;
}

auto storedManifestToJson(const std::string& content_type, const std::string& content_id) -> json {
    try {
        std::string manifest_content =
            utils::filesystem::loadFile(getPathToContentDirectory(content_type, content_id) / "manifest.json");
        return json::parse(manifest_content);
    } catch (json::parse_error& e) {
        logger::logError("content_manifest", "Failed to parse manifest: " + std::string(e.what()));
        return json::object();
    } catch (std::exception& e) {
        logger::logError("content_manifest", "Failed to load manifest: " + std::string(e.what()));
        return json::object();
    }
}

auto getEntryField(const json& file_info, const std::string& key) -> json {
    return file_info.contains(key) ? file_info[key] : json{};
}

} // namespace

ContentManifest::ContentManifest(std::string content_type, std::string content_id)
    : content_type_(std::move(content_type)), content_id_(std::move(content_id)),
      content_scanner_(std::make_unique<ContentScanner>(content_type_, content_id_)) {}

auto ContentManifest::getContentType() const -> const std::string& { return content_type_; }

auto ContentManifest::getContentId() const -> const std::string& { return content_id_; }

auto ContentManifest::toString() const -> std::string {
    updateManifest();
    return utils::filesystem::loadFile(getPathToManifestFile());
}

auto ContentManifest::toJson() const -> json {
    updateManifest();
    return storedManifestToJson(content_type_, content_id_);
}

auto ContentManifest::getPathToManifestFile() const -> fs::path {
    return getPathToContentDirectory(content_type_, content_id_) / "manifest.json";
}

auto ContentManifest::updateManifest() const -> void {
    utils::filesystem::storeFile(getPathToManifestFile(), content_scanner_->scanContent().dump(4));
}

auto getManifestSignature(const std::string& content_type, const std::string& content_id) -> std::string {
    return getPathToContentDirectory(content_type, content_id).string();
}

auto compareManifests(const ContentManifest& old_manifest, const json& new_manifest) -> std::vector<ContentChange> {
    return compareManifests(old_manifest.toJson(), new_manifest);
}

auto compareManifests(const json& old_manifest, const json& new_manifest) -> std::vector<ContentChange> {
    std::vector<ContentChange> changes;

    // Check for added or modified files
    if (new_manifest.contains("files")) {
        for (const auto& [file_path, new_file_info] : new_manifest["files"].items()) {
            if (!old_manifest.contains("files") || !old_manifest["files"].contains(file_path)) {
                changes.push_back({.type = ContentChange::Type::ADDED, .file_path = file_path});
            } else {
                const auto& old_file_info = old_manifest["files"][file_path];
                if (getEntryField(old_file_info, "sha256") != getEntryField(new_file_info, "sha256")) {
                    changes.push_back({.type = ContentChange::Type::MODIFIED, .file_path = file_path});
                } else if (getEntryField(old_file_info, "permissions") != getEntryField(new_file_info, "permissions")) {
                    changes.push_back({.type = ContentChange::Type::ATTRIBUTES_CHANGED, .file_path = file_path});
                }
            }
        }
    }

    // Check for removed files
    if (old_manifest.contains("files")) {
        for (const auto& [file_path, old_file_info] : old_manifest["files"].items()) {
            if (!new_manifest.contains("files") || !new_manifest["files"].contains(file_path)) {
                changes.push_back({.type = ContentChange::Type::REMOVED, .file_path = file_path});
            }
        }
    }

    return changes;
}

auto getPermissionsFromManifest(const json& manifest, const fs::path& file_path) -> fs::perms {
    if (manifest.contains("files") && manifest["files"].contains(file_path.string()) &&
        manifest["files"][file_path.string()].contains("permissions")) {
        return static_cast<fs::perms>(manifest["files"][file_path.string()]["permissions"].get<uint32_t>());
    }
    return fs::perms::unknown;
}

} // namespace oink_judge::content_service
