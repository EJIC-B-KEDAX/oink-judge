#pragma once
#include <nlohmann/json.hpp>
#include <string>

namespace oink_judge::services::data_sender {

using nlohmann::json;

class ContentManifest {
  public:
    ContentManifest(const std::string& content_type, const std::string& content_id);
    const std::string& get_content_type() const;
    const std::string& get_content_id() const;

    std::string to_string() const;
    json to_json() const;

    std::string get_path_to_manifest_file() const;

  private:
    std::string _content_type;
    std::string _content_id;

    time_t full_rescan_interval;
    mutable time_t _last_updated;
    mutable time_t _last_full_rescan;

    void update_manifest() const;
    void full_rescan_content() const;
    void fast_rescan_content() const;
    json stored_manifest_to_json() const;
};

std::string get_manifest_signature(const std::string& content_type, const std::string& content_id);

struct ContentChange {
    enum class Type { Added, Removed, Modified };
    Type type;
    std::string file_path;
};

std::vector<ContentChange> compare_manifests(const ContentManifest& old_manifest, const json& new_manifest);
std::vector<ContentChange> compare_manifests(const json& old_manifest, const json& new_manifest);

} // namespace oink_judge::services::data_sender
