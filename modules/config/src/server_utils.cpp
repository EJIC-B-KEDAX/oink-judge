#include "oink_judge/config/server_utils.h"

#include <oink_judge/config/config.h>
#include <oink_judge/logger/logger.h>

#include <optional>

namespace oink_judge::config {

using nlohmann::json;

auto getMyPort() -> std::optional<int> {
    const auto& config_data = config::Config::config();
    if (!config::checkObjectIsNumberInteger(config_data, {"my_port"})) {
        logger::logError("config", "Invalid or missing 'my_port' in configuration.");
        return std::nullopt;
    }

    return config_data["my_port"].get<int>();
}

} // namespace oink_judge::config
