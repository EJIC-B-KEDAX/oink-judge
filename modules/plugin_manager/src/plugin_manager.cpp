#include "oink_judge/plugin_manager/plugin_manager.h"

#include <dlfcn.h>
#include <oink_judge/logger/logger.h>

namespace oink_judge::plugin_manager {

PluginManager::PluginManager() = default;

PluginManager::~PluginManager() {
    for (auto* handle : handles_) {
        dlclose(handle);
    }
}

auto PluginManager::load(const fs::path& path) -> bool {
    logger::logInfo("plugin_manager", "Loading plugin: " + path.string());
    auto* handle = dlopen(path.c_str(), RTLD_NOW | RTLD_GLOBAL);
    if (handle == nullptr) {
        logger::logError("plugin_manager", "Failed to load plugin: " + std::string(dlerror()));
        return false;
    }
    handles_.push_back(handle);

    using RegisterTypesFunc = void (*)();
    auto* register_types = reinterpret_cast<RegisterTypesFunc>(dlsym(handle, "registerTypes")); // NOLINT
    if (register_types == nullptr) {
        logger::logError("plugin_manager", "Failed to find registerTypes function in plugin: " + std::string(dlerror()));
        dlclose(handle);
        return false;
    }
    register_types();
    return true;
}

} // namespace oink_judge::plugin_manager