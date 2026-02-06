#pragma once
#include <ctime>
#include <nlohmann/json.hpp>
#include <string>

namespace oink_judge::content_service {

using nlohmann::json;

class ContentManifest {
  public:
    ContentManifest(std::string content_type, std::string content_id);
    auto getContentType() const -> const std::string&;
    auto getContentId() const -> const std::string&;

    auto toString() const -> std::string;
    auto toJson() const -> json;

    auto getPathToManifestFile() const -> std::filesystem::path;

  private:
    std::string content_type_;
    std::string content_id_;

    time_t full_rescan_interval_;
    mutable time_t last_updated_;
    mutable time_t last_full_rescan_;

    auto updateManifest() const -> void;
    auto fullRescanContent() const -> void;
    auto fastRescanContent() const -> void;
    auto storedManifestToJson() const -> json;
};

auto getManifestSignature(const std::string& content_type, const std::string& content_id) -> std::string;

struct ContentChange {
    enum class Type : uint8_t { ADDED, REMOVED, MODIFIED };
    Type type;
    std::filesystem::path file_path;
};

auto compareManifests(const ContentManifest& old_manifest, const json& new_manifest) -> std::vector<ContentChange>;
auto compareManifests(const json& old_manifest, const json& new_manifest) -> std::vector<ContentChange>;

} // namespace oink_judge::content_service
