#pragma once
#include <optional>
#include <string>

namespace oink_judge::config {

auto getMyPort() -> std::optional<int>;

auto getConnectionHandlerType() -> std::optional<std::string>;

struct ConnectionConfig {
    std::string host;
    short port;
    std::string session_type;
    std::string start_message;
};

auto getConnectionConfig(const std::string& key) -> std::optional<ConnectionConfig>;

} // namespace oink_judge::config
