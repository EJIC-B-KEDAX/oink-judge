#include "oink_judge/database/config_utils.h"

#include <oink_judge/config/config.h>

namespace oink_judge::database {

using nlohmann::json;

auto getDatabaseConfig() -> std::optional<DatabaseConfig> {
    const auto& config_data = config::Config::config();
    const auto& credentials_data = config::Config::credentials();

    if (!config::checkObjectIsString(credentials_data, {"database", "password"})) {

        return std::nullopt;
    }

    if (!config::checkObjectIsObject(config_data, {"database"})) {
        return std::nullopt;
    }

    const auto& db_config = config_data["database"];

    if (!config::checkObjectIsString(db_config, {"host"}) || !config::checkObjectIsNumberInteger(db_config, {"port"}) ||
        !config::checkObjectIsString(db_config, {"username"}) || !config::checkObjectIsString(db_config, {"dbname"})) {
        return std::nullopt;
    }

    DatabaseConfig result;
    result.host = db_config["host"].get<std::string>();
    result.port = db_config["port"].get<int>();
    result.username = db_config["username"].get<std::string>();
    result.password = credentials_data["database"]["password"].get<std::string>();
    result.database_name = db_config["dbname"].get<std::string>();

    return result;
}

} // namespace oink_judge::database
