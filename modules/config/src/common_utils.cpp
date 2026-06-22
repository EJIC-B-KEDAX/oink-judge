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

auto getTokenFromCredentials(const std::string& path_to_token) -> std::optional<std::string> {
    nlohmann::json now_part = Config::credentials();

    std::string now_transition;
    for (char c : path_to_token) {
        if (c == '.') {
            if (!now_part.is_object() || !now_part.contains(now_transition)) {
                return std::nullopt;
            }
            now_part = now_part[now_transition];
            now_transition.clear();
        } else {
            now_transition += c;
        }
    }

    if (!now_part.is_object() || !now_part.contains(now_transition) || !now_part[now_transition].is_string()) {
        return std::nullopt;
    }
    return now_part[now_transition].get<std::string>();
}

} // namespace oink_judge::config
