#include "oink_judge/content_service/config_utils.h"

#include <oink_judge/config/common_utils.h>

namespace oink_judge::content_service {

auto getContentDirectory(const std::string& content_type) -> std::optional<fs::path> {
    return config::getDirectoryPath(content_type + "s");
}

} // namespace oink_judge::content_service
