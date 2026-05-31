#pragma once
#include <optional>
#include <string>

namespace oink_judge::socket {

auto getServerPort(const std::string& server_name) -> std::optional<int>;

auto getServerHostname(const std::string& server_name) -> std::optional<std::string>;

auto getSessionType(const std::string& session_for) -> std::optional<std::string>;

auto getStartMessage(const std::string& session_for) -> std::optional<std::string>;

struct ConnectionConfig {
    std::string host;
    short port;
    std::string session_type;
    std::string start_message;
};

auto getConnectionConfig(const std::string& key) -> std::optional<ConnectionConfig>;

} // namespace oink_judge::socket
