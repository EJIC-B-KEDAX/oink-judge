#include "oink_judge/content_service/content_manifest.h"

#include <ctime>
#include <filesystem>
#include <oink_judge/config/config.h>
#include <oink_judge/utils/crypto.h>
#include <oink_judge/utils/filesystem.h>

namespace oink_judge::content_service {

using Config = config::Config;

using namespace oink_judge::utils;

namespace {

auto getPathToContentDirectory(const std::string& content_type, const std::string& content_id) -> std::filesystem::path {
    std::string path_to_content = Config::config().at("directories").at(content_type + "s");
    return path_to_content + "/" + content_id + "/";
}

} // namespace

ContentManifest::ContentManifest(std::string content_type, std::string content_id)
    : content_type_(std::move(content_type)), content_id_(std::move(content_id)), last_updated_(std::time(nullptr)),
      last_full_rescan_(std::time(nullptr)),
      full_rescan_interval_(Config::config().at("bounds").at("full_rescan_interval").get<time_t>()) {}

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
    std::string path_to_content = Config::config().at("directories").at(content_type_ + "s");
    return path_to_content + "/" + content_id_ + "/" + "manifest.json";
}

auto ContentManifest::updateManifest() const -> void {
    time_t started_update_on = std::time(nullptr);
    if (std::difftime(std::time(nullptr), last_full_rescan_) >= static_cast<double>(full_rescan_interval_)) {
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
            std::string relative_path = std::filesystem::relative(entry.path(), content_directory).string();
            if (relative_path == "manifest.json") {
                continue;
            }
            json cur_file_json;
            cur_file_json["size"] = entry.file_size();
            cur_file_json["last_modified"] =
                std::chrono::duration_cast<std::chrono::seconds>(entry.last_write_time().time_since_epoch()).count();
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
            std::string relative_path = std::filesystem::relative(entry.path(), content_directory).string();
            if (relative_path == "manifest.json") {
                continue;
            }
            json cur_file_json;
            cur_file_json["size"] = entry.file_size();
            cur_file_json["last_modified"] =
                std::chrono::duration_cast<std::chrono::seconds>(entry.last_write_time().time_since_epoch()).count();
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

} // namespace oink_judge::content_service
