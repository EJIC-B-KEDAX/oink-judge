#include "oink_judge/socket/server_config_utils.h"

#include <oink_judge/config/config.h>
#include <oink_judge/logger/logger.h>

#include <optional>

namespace oink_judge::socket {

using nlohmann::json;

auto getMyPort() -> std::optional<int> {
    const auto& config_data = config::Config::config();
    if (!config::checkObjectIsNumberInteger(config_data, {"my_port"})) {
        logger::logError("socket", "Invalid or missing 'my_port' in configuration.");
        return std::nullopt;
    }

    return config_data["my_port"].get<int>();
}

auto getConnectionHandlerType() -> std::optional<std::string> {
    const auto& config_data = config::Config::config();
    if (!config::checkObjectIsString(config_data, {"connection_handler_type"})) {
        return std::nullopt;
    }

    return config_data["connection_handler_type"].get<std::string>();
}

} // namespace oink_judge::socket
