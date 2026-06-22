#include "oink_judge/content_service/content_scanner.h"

#include "oink_judge/content_service/config_utils.h"

#include <oink_judge/config/common_utils.h>
#include <oink_judge/logger/logger.h>
#include <oink_judge/utils/crypto.h>
#include <oink_judge/utils/filesystem.h>

#include <cstdint>

namespace oink_judge::content_service {

using config::requireHasValue;
namespace fs = std::filesystem;

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

} // namespace

ContentScanner::ContentScanner(std::string content_type, std::string content_id)
    : content_type_(std::move(content_type)), content_id_(std::move(content_id)),
      full_rescan_interval_(requireHasValue(config::getTiming("full_rescan_interval"))),
      last_full_rescan_(std::chrono::duration_cast<std::chrono::milliseconds>(
          std::chrono::system_clock::time_point::min().time_since_epoch())) {}

auto ContentScanner::scanContent() -> json {
    constexpr int64_t SECONDS_TO_MILLISECONDS = 1000;
    if (std::chrono::duration_cast<std::chrono::milliseconds>(std::chrono::system_clock::now().time_since_epoch()).count() -
            std::chrono::duration_cast<std::chrono::milliseconds>(last_full_rescan_).count() >=
        static_cast<int64_t>(full_rescan_interval_.count() * SECONDS_TO_MILLISECONDS)) {
        return fullContentScan();
    }
    return scanContentWithCache();
}

auto ContentScanner::fullContentScan() -> json {
    fs::path content_directory = getPathToContentDirectory(content_type_, content_id_);
    utils::filesystem::createDirectoryIfNotExists(content_directory);
    json manifest_json;

    for (const auto& entry : fs::recursive_directory_iterator(content_directory)) {
        if (entry.is_regular_file()) {
            logger::logDebug("content_scanner", "Processing file: " + entry.path().string(), 3);
            std::string relative_path = fs::relative(entry.path(), content_directory).string();
            if (relative_path == "manifest.json") {
                continue;
            }
            json cur_file_json;
            cur_file_json["size"] = entry.file_size();
            cur_file_json["last_modified"] =
                std::chrono::duration_cast<std::chrono::seconds>(entry.last_write_time().time_since_epoch()).count();
            cur_file_json["permissions"] = static_cast<uint32_t>(entry.status().permissions());
            cur_file_json["sha256"] = utils::crypto::sha256(utils::filesystem::loadFile(entry.path()));
            manifest_json["files"][relative_path] = cur_file_json;
        }
    }
    last_full_rescan_ =
        std::chrono::duration_cast<std::chrono::milliseconds>(std::chrono::system_clock::now().time_since_epoch());

    return manifest_json;
}

auto ContentScanner::scanContentWithCache() -> json {
    fs::path content_directory = getPathToContentDirectory(content_type_, content_id_);
    utils::filesystem::createDirectoryIfNotExists(content_directory);
    json new_manifest_json;
    json stored_manifest_cache = storedManifestToJson(content_type_, content_id_);

    for (const auto& entry : fs::recursive_directory_iterator(content_directory)) {
        if (entry.is_regular_file()) {
            logger::logDebug("content_scanner", "Processing file: " + entry.path().string(), 3);
            std::string relative_path = fs::relative(entry.path(), content_directory).string();
            if (relative_path == "manifest.json") {
                continue;
            }
            json cur_file_json;
            cur_file_json["size"] = entry.file_size();
            cur_file_json["last_modified"] =
                std::chrono::duration_cast<std::chrono::seconds>(entry.last_write_time().time_since_epoch()).count();
            cur_file_json["permissions"] = static_cast<uint32_t>(entry.status().permissions());
            if (stored_manifest_cache.contains("files") && stored_manifest_cache["files"].contains(relative_path) &&
                stored_manifest_cache["files"][relative_path]["size"] == cur_file_json["size"] &&
                stored_manifest_cache["files"][relative_path]["last_modified"] == cur_file_json["last_modified"]) {

                // File unchanged, copy sha256 from stored manifest
                cur_file_json["sha256"] = stored_manifest_cache["files"][relative_path]["sha256"];
            } else {
                cur_file_json["sha256"] = utils::crypto::sha256(utils::filesystem::loadFile(entry.path()));
            }
            new_manifest_json["files"][relative_path] = cur_file_json;
        }
    }

    return new_manifest_json;
}

} // namespace oink_judge::content_service
