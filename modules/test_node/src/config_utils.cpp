#include "oink_judge/test_node/config_utils.h"

#include <oink_judge/config/config.h>

namespace oink_judge::test_node {

using config::Config;

auto getMyTestNodeId() -> std::optional<std::string> {
    const auto& config_data = Config::config();
    if (!config::checkObjectIsString(config_data, {"my_id"})) {
        return std::nullopt;
    }

    return config_data["my_id"].get<std::string>();
}

auto getMyTestNodeType() -> std::optional<std::string> {
    const auto& config_data = Config::config();
    if (!config::checkObjectIsString(config_data, {"my_type"})) {
        return std::nullopt;
    }

    return config_data["my_type"].get<std::string>();
}

auto getTestingLogFilePath(const std::string& key) -> std::optional<fs::path> {
    const auto& config_data = Config::config();
    if (!config::checkObjectIsString(config_data, {"testing", key})) {
        return std::nullopt;
    }

    return std::filesystem::path(config_data["testing"][key].get<std::string>());
}

auto getQueueManagerServiceStubType() -> std::optional<std::string> {
    const auto& config_data = Config::config();
    if (!config::checkObjectIsString(config_data, {"queue_manager_service", "stub_type"})) {
        return std::nullopt;
    }

    return config_data["queue_manager_service"]["stub_type"].get<std::string>();
}

} // namespace oink_judge::test_node
