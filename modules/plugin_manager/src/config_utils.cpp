#include "oink_judge/plugin_manager/config_utils.h"

#include <oink_judge/config/config.h>
#include <oink_judge/logger/logger.h>

namespace oink_judge::plugin_manager {

using logger::requireHasValue;

auto getAllPluginPaths() -> std::vector<fs::path> {
    std::vector<fs::path> plugin_paths;
    try {
        auto plugins_dir = requireHasValue(config::getDirectoryPath("plugins"));
        if (fs::exists(plugins_dir) && fs::is_directory(plugins_dir)) {
            for (const auto& entry : fs::directory_iterator(plugins_dir)) {
                if (entry.is_regular_file() && entry.path().extension() == ".so") {
                    plugin_paths.push_back(entry.path());
                }
            }
        }
        return plugin_paths;
    } catch (const std::exception& e) {
        logger::logError("plugin_manager", e.what());
        return {};
    }
}

} // namespace oink_judge::plugin_manager