#include "oink_judge/config/server_config_utils.h"

#include "oink_judge/config/config.h"

#include <optional>

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

auto getConnectionConfig(const std::string& key) -> std::optional<ConnectionConfig> {
    std::optional<std::string> host_opt = getServerHostname(key);
    if (!host_opt) {
        return std::nullopt;
    }

    std::optional<short> port_opt = getServerPort(key);
    if (!port_opt) {
        return std::nullopt;
    }

    std::optional<std::string> session_type_opt = getSessionType(key);
    if (!session_type_opt) {
        return std::nullopt;
    }

    std::optional<std::string> start_message_opt = getStartMessage(key);
    if (!start_message_opt) {
        return std::nullopt;
    }

    return ConnectionConfig(*host_opt, *port_opt, *session_type_opt, *start_message_opt);
}

} // namespace oink_judge::config
