#include "oink_judge/config/common_utils.h"

#include "oink_judge/config/config.h"

namespace oink_judge::config {

auto getDirectoryPath(const std::string& key) -> std::optional<fs::path> {
    const auto& config_data = Config::config();

    if (!checkObjectIsString(config_data, {"directories", key})) {
        return std::nullopt;
    }

    return fs::path(config_data["directories"][key].get<std::string>());
}

auto getTiming(const std::string& timing_name) -> std::optional<std::chrono::duration<double>> {
    const auto& config_data = Config::config();

    if (!checkObjectIsNumber(config_data, {"timings", timing_name})) {
        return std::nullopt;
    }

    double seconds = config_data["timings"][timing_name].get<double>();
    return std::chrono::duration<double>(seconds);
}

} // namespace oink_judge::config
