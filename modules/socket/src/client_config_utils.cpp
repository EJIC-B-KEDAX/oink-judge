#include "oink_judge/socket/client_config_utils.h"

#include <oink_judge/config/config.h>

namespace oink_judge::socket {

using nlohmann::json;

auto getServerPort(const std::string& server_name) -> std::optional<int> {
    const auto& config_data = config::Config::config();
    if (!config::checkObjectIsNumberInteger(config_data, {"ports", server_name})) {
        return std::nullopt;
    }

    return config_data["ports"][server_name].get<int>();
}

auto getServerHostname(const std::string& server_name) -> std::optional<std::string> {
    const auto& config_data = config::Config::config();
    if (!config::checkObjectIsString(config_data, {"hosts", server_name})) {
        return std::nullopt;
    }

    return config_data["hosts"][server_name].get<std::string>();
}

auto getSessionType(const std::string& session_for) -> std::optional<std::string> {
    const auto& config_data = config::Config::config();
    if (!config::checkObjectIsString(config_data, {"sessions", session_for})) {
        return std::nullopt;
    }

    return config_data["sessions"][session_for].get<std::string>();
}

auto getStartMessage(const std::string& session_for) -> std::optional<std::string> {
    const auto& config_data = config::Config::config();
    if (!config::checkObjectIsString(config_data, {"start_messages", session_for})) {
        return std::nullopt;
    }

    return config_data["start_messages"][session_for].get<std::string>();
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

} // namespace oink_judge::socket
