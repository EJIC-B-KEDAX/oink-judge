#include "oink_judge/content_service/content_manifest.h"

#include "oink_judge/content_service/config_utils.h"

#include <ctime>
#include <filesystem>
#include <oink_judge/config/common_utils.h>
#include <oink_judge/logger/logger.h>
#include <oink_judge/utils/crypto.h>
#include <oink_judge/utils/filesystem.h>

namespace oink_judge::content_service {

using config::requireHasValue;

using namespace oink_judge::utils;

namespace {

auto getPathToContentDirectory(const std::string& content_type, const std::string& content_id) -> std::filesystem::path {
    std::filesystem::path path_to_content = requireHasValue(getContentDirectory(content_type));
    return path_to_content / content_id;
}

} // namespace

ContentManifest::ContentManifest(std::string content_type, std::string content_id)
    : content_type_(std::move(content_type)), content_id_(std::move(content_id)), last_updated_(std::time(nullptr)),
      last_full_rescan_(std::time(nullptr)), full_rescan_interval_(requireHasValue(config::getTiming("full_rescan_interval"))) {}

auto ContentManifest::getContentType() const -> const std::string& { return content_type_; }

auto ContentManifest::getContentId() const -> const std::string& { return content_id_; }

auto ContentManifest::toString() const -> std::string {
    updateManifest();
    return filesystem::loadFile(getPathToManifestFile());
}

auto ContentManifest::toJson() const -> json {
    updateManifest();
    return storedManifestToJson();
}

auto ContentManifest::getPathToManifestFile() const -> std::filesystem::path {
    fs::path path_to_content = requireHasValue(getContentDirectory(content_type_));
    return path_to_content / content_id_ / "manifest.json";
}

auto ContentManifest::updateManifest() const -> void {
    time_t started_update_on = std::time(nullptr);
    if (std::difftime(std::time(nullptr), last_full_rescan_) >= full_rescan_interval_.count()) {
        last_full_rescan_ = std::time(nullptr);
        fullRescanContent();
    } else {
        fastRescanContent();
    }
    last_updated_ = started_update_on;
}

auto ContentManifest::fullRescanContent() const -> void {
    filesystem::createDirectoryIfNotExists(getPathToContentDirectory(content_type_, content_id_));
    json manifest_json;

    std::filesystem::path content_directory = getPathToContentDirectory(content_type_, content_id_);

    for (const auto& entry : std::filesystem::recursive_directory_iterator(content_directory)) {
        if (entry.is_regular_file()) {
            logger::logInfo("content_manifest", "Processing file: " + entry.path().string());
            std::string relative_path = std::filesystem::relative(entry.path(), content_directory).string();
            if (relative_path == "manifest.json") {
                continue;
            }
            json cur_file_json;
            cur_file_json["size"] = entry.file_size();
            cur_file_json["last_modified"] =
                std::chrono::duration_cast<std::chrono::seconds>(entry.last_write_time().time_since_epoch()).count();
            cur_file_json["permissions"] = static_cast<uint32_t>(entry.status().permissions());
            cur_file_json["sha256"] = crypto::sha256(filesystem::loadFile(entry.path()));
            manifest_json["files"][relative_path] = cur_file_json;
        }
    }

    filesystem::storeFile(getPathToManifestFile(), manifest_json.dump(4));
}

auto ContentManifest::fastRescanContent() const -> void {
    filesystem::createDirectoryIfNotExists(getPathToContentDirectory(content_type_, content_id_));
    json new_manifest_json;

    json stored_manifest = storedManifestToJson();
    std::filesystem::path content_directory = getPathToContentDirectory(content_type_, content_id_);

    for (const auto& entry : std::filesystem::recursive_directory_iterator(content_directory)) {
        if (entry.is_regular_file()) {
            logger::logInfo("content_manifest", "Processing file: " + entry.path().string());
            std::string relative_path = std::filesystem::relative(entry.path(), content_directory).string();
            if (relative_path == "manifest.json") {
                continue;
            }
            json cur_file_json;
            cur_file_json["size"] = entry.file_size();
            cur_file_json["last_modified"] =
                std::chrono::duration_cast<std::chrono::seconds>(entry.last_write_time().time_since_epoch()).count();
            cur_file_json["permissions"] = static_cast<uint32_t>(entry.status().permissions());
            if (stored_manifest.contains("files") && stored_manifest["files"].contains(relative_path) &&
                stored_manifest["files"][relative_path]["size"] == cur_file_json["size"] &&
                stored_manifest["files"][relative_path]["last_modified"] == cur_file_json["last_modified"]) {

                // File unchanged, copy sha256 from stored manifest
                cur_file_json["sha256"] = stored_manifest["files"][relative_path]["sha256"];
            } else {
                cur_file_json["sha256"] = crypto::sha256(filesystem::loadFile(entry.path()));
            }
            new_manifest_json["files"][relative_path] = cur_file_json;
        }
    }

    filesystem::storeFile(getPathToManifestFile(), new_manifest_json.dump(4));
}

auto ContentManifest::storedManifestToJson() const -> json {
    try {
        std::string manifest_content = filesystem::loadFile(getPathToManifestFile());
        return json::parse(manifest_content);
    } catch (...) {
        return json::object();
    }
}

auto getManifestSignature(const std::string& content_type, const std::string& content_id) -> std::string {
    return getPathToContentDirectory(content_type, content_id);
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
                changes.push_back({ContentChange::Type::ADDED, file_path});
            } else {
                const auto& old_file_info = old_manifest["files"][file_path];
                if (old_file_info["sha256"] != new_file_info["sha256"]) {
                    changes.push_back({ContentChange::Type::MODIFIED, file_path});
                } else if (old_file_info["permissions"] != new_file_info["permissions"]) {
                    changes.push_back({ContentChange::Type::ATTRIBUTES_CHANGED, file_path});
                }
            }
        }
    }

    // Check for removed files
    if (old_manifest.contains("files")) {
        for (const auto& [file_path, old_file_info] : old_manifest["files"].items()) {
            if (!new_manifest.contains("files") || !new_manifest["files"].contains(file_path)) {
                changes.push_back({ContentChange::Type::REMOVED, file_path});
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
