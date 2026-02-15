#pragma once
#include <filesystem>
#include <vector>

namespace oink_judge::plugin_manager {

namespace fs = std::filesystem;

class PluginManager {
  public:
    PluginManager();
    PluginManager(const PluginManager&) = delete;
    auto operator=(const PluginManager&) -> PluginManager& = delete;
    PluginManager(PluginManager&&) = delete;
    auto operator=(PluginManager&&) -> PluginManager& = delete;
    ~PluginManager();

    auto load(const fs::path& path) -> bool;

  private:
    std::vector<void*> handles_;
};

} // namespace oink_judge::plugin_manager
