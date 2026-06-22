#include "oink_judge/content_service/manifest_storage.h"

namespace oink_judge::content_service {

auto ManifestStorage::instance() -> ManifestStorage& {
    static ManifestStorage instance;
    return instance;
}

auto ManifestStorage::getManifest(const std::string& content_type, const std::string& content_id) -> const ContentManifest& {
    std::lock_guard<std::mutex> lock(mutex_);
    std::string key = getManifestSignature(content_type, content_id);
    auto it = manifests_.find(key);
    if (it != manifests_.end()) {
        return it->second;
    }
    auto [iter, inserted] = manifests_.emplace(key, ContentManifest(content_type, content_id));
    if (!inserted) {
        throw std::runtime_error("Failed to insert new manifest into storage");
    }
    return iter->second;
}

ManifestStorage::ManifestStorage() = default;

} // namespace oink_judge::content_service
