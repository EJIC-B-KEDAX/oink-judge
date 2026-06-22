#pragma once
#include <optional>
#include <string>

namespace oink_judge::database {

struct DatabaseConfig {
    std::string host;
    int port;
    std::string username;
    std::string password;
    std::string database_name;
    int pool_min = 2;
    int pool_max = 10;            // NOLINT
    int connect_timeout_sec = 10; // NOLINT
};

auto getDatabaseConfig() -> std::optional<DatabaseConfig>;

auto buildConnectionString(const DatabaseConfig& config) -> std::string;

} // namespace oink_judge::database
