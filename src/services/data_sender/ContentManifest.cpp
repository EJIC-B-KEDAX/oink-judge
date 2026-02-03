#include "services/data_sender/ContentManifest.h"
#include "config/Config.h"
#include "utils/filesystem.h"
#include "utils/crypto.h"
#include <ctime>
#include <fstream>
#include <filesystem>
#include <iostream>

namespace oink_judge::services::data_sender {

using Config = config::Config;

using namespace oink_judge::utils;

namespace {

std::string get_path_to_content_directory(const std::string &content_type, const std::string &content_id) {
    std::string path_to_content = Config::config().at("directories").at(content_type + "s");
    return path_to_content + "/" + content_id + "/";
}

} // namespace


ContentManifest::ContentManifest(const std::string& content_type, const std::string& content_id)
    : _content_type(content_type), _content_id(content_id), _last_updated(std::time(nullptr)),
      _last_full_rescan(std::time(nullptr)), full_rescan_interval(Config::config().at("bounds").at("full_rescan_interval").get<time_t>()) {}

const std::string& ContentManifest::get_content_type() const {
    return _content_type;
}

const std::string& ContentManifest::get_content_id() const {
    return _content_id;
}

std::string ContentManifest::to_string() const {
    update_manifest();
    return filesystem::load_file(get_path_to_manifest_file());
}

json ContentManifest::to_json() const {
    update_manifest();
    std::cerr << "Loading updated manifest: " << stored_manifest_to_json().dump() << std::endl;
    return stored_manifest_to_json();
}

std::string ContentManifest::get_path_to_manifest_file() const {
    std::string path_to_content = Config::config().at("directories").at(_content_type + "s");
    return path_to_content + "/" + _content_id + "/" + "manifest.json";
}

void ContentManifest::update_manifest() const {
    std::cerr << "Updating manifest for " << _content_type << " " << _content_id << std::endl;
    time_t started_update_on = std::time(nullptr);
    std::cerr << "Time since last full rescan: " << std::difftime(std::time(nullptr), _last_full_rescan) << " seconds" << std::endl;
    if (std::difftime(std::time(nullptr), _last_full_rescan) >= full_rescan_interval) {
        std::cerr << "Performing full rescan based on interval of " << full_rescan_interval << " seconds" << std::endl;
        _last_full_rescan = std::time(nullptr);
        full_rescan_content();
    } else {
        std::cerr << "Performing fast rescan" << std::endl;
        fast_rescan_content();
    }
    _last_updated = started_update_on;
}

void ContentManifest::full_rescan_content() const {
    filesystem::create_directory_if_not_exists(
        get_path_to_content_directory(_content_type, _content_id)
    );
    json manifest_json;

    const std::string content_directory = get_path_to_content_directory(_content_type, _content_id);

    std::cerr << "Performing full rescan of content directory: " << content_directory << std::endl;

    for (const auto &entry : std::filesystem::recursive_directory_iterator(content_directory)) {
        std::cerr << "Checking entry: " << entry.path() << std::endl;
        if (entry.is_regular_file()) {
            const std::string relative_path = std::filesystem::relative(entry.path(), content_directory).string();
            if (relative_path == "manifest.json") {
                continue;
            }
            json cur_file_json;
            cur_file_json["size"] = entry.file_size();
            cur_file_json["last_modified"] = std::chrono::duration_cast<std::chrono::seconds>(
                entry.last_write_time().time_since_epoch()).count();
            cur_file_json["sha256"] = crypto::sha256(filesystem::load_file(entry.path().string()));
            manifest_json["files"][relative_path] = cur_file_json;
        }
    }

    filesystem::store_file(get_path_to_manifest_file(), manifest_json.dump(4));
}

void ContentManifest::fast_rescan_content() const {
    filesystem::create_directory_if_not_exists(
        get_path_to_content_directory(_content_type, _content_id)
    );
    json new_manifest_json;

    json stored_manifest = stored_manifest_to_json();
    const std::string content_directory = get_path_to_content_directory(_content_type, _content_id);

    std::cerr << "Performing fast rescan of content directory: " << content_directory << std::endl;

    for (const auto &entry : std::filesystem::recursive_directory_iterator(content_directory)) {
        std::cerr << "Checking entry: " << entry.path() << std::endl;
        if (entry.is_regular_file()) {
            const std::string relative_path = std::filesystem::relative(entry.path(), content_directory).string();
            if (relative_path == "manifest.json") {
                continue;
            }
            json cur_file_json;
            cur_file_json["size"] = entry.file_size();
            cur_file_json["last_modified"] = std::chrono::duration_cast<std::chrono::seconds>(
                entry.last_write_time().time_since_epoch()).count();
            if (stored_manifest.contains("files")
                && stored_manifest["files"].contains(relative_path)
                && stored_manifest["files"][relative_path]["size"] == cur_file_json["size"]
                && stored_manifest["files"][relative_path]["last_modified"] == cur_file_json["last_modified"] ) {

                // File unchanged, copy sha256 from stored manifest
                cur_file_json["sha256"] = stored_manifest["files"][relative_path]["sha256"];
            } else {
                cur_file_json["sha256"] = crypto::sha256(filesystem::load_file(entry.path().string()));
            }
            new_manifest_json["files"][relative_path] = cur_file_json;
        }
    }

    std::cerr << "Storing updated manifest: " << new_manifest_json.dump() << std::endl;

    filesystem::store_file(get_path_to_manifest_file(), new_manifest_json.dump(4));
}

json ContentManifest::stored_manifest_to_json() const {
    try {
        std::string manifest_content = filesystem::load_file(get_path_to_manifest_file());
        return json::parse(manifest_content);
    } catch (...) {
        return json::object();
    }
}

std::string get_manifest_signature(const std::string& content_type, const std::string& content_id) {
    return get_path_to_content_directory(content_type, content_id);
}

std::vector<ContentChange> compare_manifests(const ContentManifest& old_manifest, const json& new_manifest) {
    return compare_manifests(old_manifest.to_json(), new_manifest);
}

std::vector<ContentChange> compare_manifests(const json& old_manifest, const json& new_manifest) {
    std::cerr << "Comparing manifests..." << std::endl;
    std::vector<ContentChange> changes;

    // Check for added or modified files
    if (new_manifest.contains("files")) {
        for (auto& [file_path, new_file_info] : new_manifest["files"].items()) {
            std::cerr << "Comparing file: " << file_path << std::endl;
            if (!old_manifest.contains("files") || !old_manifest["files"].contains(file_path)) {
                changes.push_back({ContentChange::Type::Added, file_path});
            } else {
                auto& old_file_info = old_manifest["files"][file_path];
                if (old_file_info["sha256"] != new_file_info["sha256"]) {
                    changes.push_back({ContentChange::Type::Modified, file_path});
                }
            }
        }
    }

    // Check for removed files
    if (old_manifest.contains("files")) {
        for (auto& [file_path, old_file_info] : old_manifest["files"].items()) {
            if (!new_manifest.contains("files") || !new_manifest["files"].contains(file_path)) {
                changes.push_back({ContentChange::Type::Removed, file_path});
            }
        }
    }

    return changes;
}

} // namespace oink_judge::services::data_sender
