#pragma once
#include "oink_judge/content_service/content_manifest.h"

namespace oink_judge::content_service {

class ManifestStorage {
  public:
    static auto instance() -> ManifestStorage&;

    ManifestStorage(const ManifestStorage&) = delete;
    auto operator=(const ManifestStorage&) -> ManifestStorage& = delete;
    ManifestStorage(ManifestStorage&&) = delete;
    auto operator=(ManifestStorage&&) -> ManifestStorage& = delete;
    ~ManifestStorage() = default;

    auto getManifest(const std::string& content_type, const std::string& content_id) -> const ContentManifest&;

  private:
    ManifestStorage();

    std::map<std::string, ContentManifest> manifests_;
};

} // namespace oink_judge::content_service
