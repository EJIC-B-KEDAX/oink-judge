#include "oink_judge/config/server_config_utils.h"

#include "oink_judge/config/config.h"

namespace oink_judge::config {

auto getMyPort() -> std::optional<int> {
    const auto& config_data = Config::config();
    if (!config_data.contains("my_port") || !config_data["my_port"].is_number_integer()) {
        return std::nullopt;
    }

    return config_data["my_port"].get<int>();
}

auto getConnectionHandlerType() -> std::optional<std::string> {
    const auto& config_data = Config::config();
    if (!config_data.contains("connection_handler_type") || !config_data["connection_handler_type"].is_string()) {
        return std::nullopt;
    }

    return config_data["connection_handler_type"].get<std::string>();
}

} // namespace oink_judge::config
