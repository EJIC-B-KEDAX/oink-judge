#include "services/data_sender/ManifestStorage.h"

namespace oink_judge::services::data_sender {

ManifestStorage &ManifestStorage::instance() {
    static ManifestStorage instance;
    return instance;
}

const ContentManifest &ManifestStorage::get_manifest(const std::string &content_type, const std::string &content_id) {
    std::string key = get_manifest_signature(content_type, content_id);
    auto it = _manifests.find(key);
    if (it != _manifests.end()) {
        return it->second;
    } else {
        auto [it, inserted] = _manifests.emplace(key, ContentManifest(content_type, content_id));
        return it->second;
    }
}

ManifestStorage::ManifestStorage() = default;

} // namespace oink_judge::services::data_sender
