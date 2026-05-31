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
};

auto getDatabaseConfig() -> std::optional<DatabaseConfig>;

} // namespace oink_judge::database
