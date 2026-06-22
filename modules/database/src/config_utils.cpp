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

    if (config::checkObjectIsNumberInteger(db_config, {"pool_min"})) {
        result.pool_min = db_config["pool_min"].get<int>();
    }
    if (config::checkObjectIsNumberInteger(db_config, {"pool_max"})) {
        result.pool_max = db_config["pool_max"].get<int>();
    }
    if (config::checkObjectIsNumberInteger(db_config, {"connect_timeout_sec"})) {
        result.connect_timeout_sec = db_config["connect_timeout_sec"].get<int>();
    }

    if (result.pool_min < 0 || result.pool_max < 1 || result.pool_min > result.pool_max) {
        return std::nullopt;
    }

    return result;
}

auto buildConnectionString(const DatabaseConfig& config) -> std::string {
    return "host=" + config.host + " port=" + std::to_string(config.port) + " dbname=" + config.database_name +
           " user=" + config.username + " password=" + config.password +
           " keepalives=1 keepalives_idle=30 keepalives_interval=10 keepalives_count=5 connect_timeout=" +
           std::to_string(config.connect_timeout_sec) + " sslmode=require";
}

} // namespace oink_judge::database
