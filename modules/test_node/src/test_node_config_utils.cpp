#include "oink_judge/test_node/test_node_config_utils.h"

#include <oink_judge/config/config.h>

namespace oink_judge::test_node {

using config::Config;

auto getMyTestNodeId() -> std::optional<std::string> {
    const auto& config_data = Config::config();
    if (!config_data.contains("my_id") || !config_data["my_id"].is_string()) {
        return std::nullopt;
    }

    return config_data["my_id"].get<std::string>();
}

auto getTestingLogFilePath(const std::string& key) -> std::optional<std::string> {
    const auto& config_data = Config::config();
    if (!config_data.contains("testing") || !config_data["testing"].contains(key) || !config_data["testing"][key].is_string()) {
        return std::nullopt;
    }

    return std::filesystem::path(config_data["testing"][key].get<std::string>());
}

} // namespace oink_judge::test_node
