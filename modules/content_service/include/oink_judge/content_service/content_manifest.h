#pragma once
#include "oink_judge/content_service/content_scanner.h"

#include <nlohmann/json.hpp>

#include <filesystem>
#include <string>
#include <vector>

namespace oink_judge::content_service {

using nlohmann::json;

namespace fs = std::filesystem;

class ContentManifest {
  public:
    ContentManifest(std::string content_type, std::string content_id);
    [[nodiscard]] auto getContentType() const -> const std::string&;
    [[nodiscard]] auto getContentId() const -> const std::string&;

    [[nodiscard]] auto toString() const -> std::string;
    [[nodiscard]] auto toJson() const -> json;

    [[nodiscard]] auto getPathToManifestFile() const -> fs::path;

    auto updateManifest() const -> void;

  private:
    std::string content_type_;
    std::string content_id_;

    std::unique_ptr<ContentScanner> content_scanner_;
};

auto getManifestSignature(const std::string& content_type, const std::string& content_id) -> std::string;

struct ContentChange {
    enum class Type : uint8_t { ADDED, REMOVED, MODIFIED, ATTRIBUTES_CHANGED };
    Type type;
    fs::path file_path;
};

auto compareManifests(const ContentManifest& old_manifest, const json& new_manifest) -> std::vector<ContentChange>;
auto compareManifests(const json& old_manifest, const json& new_manifest) -> std::vector<ContentChange>;

auto getPermissionsFromManifest(const json& manifest, const fs::path& file_path) -> fs::perms;

} // namespace oink_judge::content_service
