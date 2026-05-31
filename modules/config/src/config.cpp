#include "oink_judge/config/config.h"

#include <oink_judge/logger/logger.h>

#include <fstream>
#include <string>

namespace oink_judge::config {

using json = nlohmann::json;

std::filesystem::path Config::config_file_path;
std::filesystem::path Config::credentials_file_path;
// Static unique_ptr instances are declared in the class and defined here.
std::unique_ptr<Config> Config::config_instance = nullptr;
std::unique_ptr<Config> Config::credentials_instance = nullptr;

auto Config::config() -> const json& {
    if (!config_instance) {
        config_instance = std::unique_ptr<Config>(new Config(config_file_path));
    }
    return config_instance->config_data_;
}

auto Config::credentials() -> const json& {
    if (!credentials_instance) {
        credentials_instance = std::unique_ptr<Config>(new Config(credentials_file_path));
    }
    return credentials_instance->config_data_;
}

auto Config::setConfigFilePath(const std::filesystem::path& path) -> void { config_file_path = path; }

auto Config::setCredentialsFilePath(const std::filesystem::path& path) -> void { credentials_file_path = path; }

auto Config::reloadData() -> void {
    config_instance = std::unique_ptr<Config>(new Config(config_file_path));
    credentials_instance = std::unique_ptr<Config>(new Config(credentials_file_path));
}

Config::Config(const std::filesystem::path& file_path) {
    std::ifstream config_file(file_path);
    if (!config_file.is_open()) {
        throw std::runtime_error("Could not open config file: " + file_path.string());
    }
    config_file >> config_data_;
}

auto checkPathWith(const json& j, const std::vector<std::string>& path, const std::function<bool(const json&)>& predicate)
    -> bool {
    const json* current = &j;
    for (const auto& key : path) {
        if (!current->is_object()) {
            return false;
        }
        if (!current->contains(key)) {
            return false;
        }
        current = &(current->at(key));
    }
    return predicate(*current);
}

auto checkPath(const json& j, const std::vector<std::string>& path, json::value_t type) -> bool {
    return checkPathWith(j, path, [&](const json& j) -> bool { return j.type() == type; });
}

auto checkObjectIsArray(const json& j, const std::vector<std::string>& path) -> bool {
    return checkPath(j, path, json::value_t::array);
}

auto checkObjectIsBinary(const json& j, const std::vector<std::string>& path) -> bool {
    return checkPath(j, path, json::value_t::binary);
}

auto checkObjectIsBoolean(const json& j, const std::vector<std::string>& path) -> bool {
    return checkPath(j, path, json::value_t::boolean);
}

auto checkObjectIsDiscarded(const json& j, const std::vector<std::string>& path) -> bool {
    return checkPath(j, path, json::value_t::discarded);
}

auto checkObjectIsNull(const json& j, const std::vector<std::string>& path) -> bool {
    return checkPath(j, path, json::value_t::null);
}

auto checkObjectIsNumber(const json& j, const std::vector<std::string>& path) -> bool {
    return checkPathWith(j, path, [](const json& j) -> bool { return j.is_number(); });
}

auto checkObjectIsNumberFloat(const json& j, const std::vector<std::string>& path) -> bool {
    return checkPath(j, path, json::value_t::number_float);
}

auto checkObjectIsNumberInteger(const json& j, const std::vector<std::string>& path) -> bool {
    return checkPathWith(j, path, [](const json& j) -> bool { return j.is_number_integer(); });
}

auto checkObjectIsNumberUnsigned(const json& j, const std::vector<std::string>& path) -> bool {
    return checkPath(j, path, json::value_t::number_unsigned);
}

auto checkObjectIsObject(const json& j, const std::vector<std::string>& path) -> bool {
    return checkPath(j, path, json::value_t::object);
}

auto checkObjectIsPrimitive(const json& j, const std::vector<std::string>& path) -> bool {
    return checkPathWith(j, path, [](const json& j) -> bool { return j.is_primitive(); });
}

auto checkObjectIsString(const json& j, const std::vector<std::string>& path) -> bool {
    return checkPath(j, path, json::value_t::string);
}

} // namespace oink_judge::config
