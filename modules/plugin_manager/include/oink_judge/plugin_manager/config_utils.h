#pragma once
#include <filesystem>
#include <vector>

namespace oink_judge::plugin_manager {

namespace fs = std::filesystem;

auto getAllPluginPaths() -> std::vector<fs::path>;

} // namespace oink_judge::plugin_manager
