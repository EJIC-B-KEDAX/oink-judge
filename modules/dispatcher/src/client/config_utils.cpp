#include "oink_judge/dispatcher/client/config_utils.h"

#include <oink_judge/config/config.h>

namespace oink_judge::dispatcher {

auto getDispatcherStubType() -> std::optional<std::string> {
    const auto& config = config::Config::config();
    if (!config::checkObjectIsString(config, {"dispatcher", "stub_type"})) {
        return std::nullopt;
    }
    return config["dispatcher"]["stub_type"].get<std::string>();
}

} // namespace oink_judge::dispatcher
