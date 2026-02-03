#pragma once
#include "ContentManifest.h"

namespace oink_judge::services::data_sender {

class ManifestStorage {
  public:
    static ManifestStorage& instance();
    ManifestStorage(const ManifestStorage&) = delete;
    ManifestStorage& operator=(const ManifestStorage&) = delete;

    const ContentManifest& get_manifest(const std::string& content_type, const std::string& content_id);

  private:
    ManifestStorage();

    std::map<std::string, ContentManifest> _manifests;
};

} // namespace oink_judge::services::data_sender
