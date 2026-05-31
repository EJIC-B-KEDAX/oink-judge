#pragma once
#include <nlohmann/json.hpp>

#include <filesystem>
#include <functional>
#include <memory>
#include <string>
#include <vector>

namespace oink_judge::config {

using nlohmann::json;

class Config {
  public:
    Config(const Config&) = delete;
    auto operator=(const Config&) -> Config& = delete;
    Config(Config&&) = delete;
    auto operator=(Config&&) -> Config& = delete;
    ~Config() = default;

    static auto config() -> const json&;
    static auto credentials() -> const json&;

    static auto setConfigFilePath(const std::filesystem::path& path) -> void;
    static auto setCredentialsFilePath(const std::filesystem::path& path) -> void;

    static auto reloadData() -> void;

  private:
    static std::unique_ptr<Config> config_instance;
    static std::unique_ptr<Config> credentials_instance;
    static std::filesystem::path config_file_path;      // Path to the main configuration file, must be set before use
    static std::filesystem::path credentials_file_path; // Path to the main credentials file, must be set before use
    Config(const std::filesystem::path& file_path);

    json config_data_;
};

auto checkPathWith(const json& j, const std::vector<std::string>& path, const std::function<bool(const json&)>& predicate)
    -> bool;
auto checkPath(const json& j, const std::vector<std::string>& path, json::value_t type) -> bool;

auto checkObjectIsArray(const json& j, const std::vector<std::string>& path) -> bool;
auto checkObjectIsBinary(const json& j, const std::vector<std::string>& path) -> bool;
auto checkObjectIsBoolean(const json& j, const std::vector<std::string>& path) -> bool;
auto checkObjectIsDiscarded(const json& j, const std::vector<std::string>& path) -> bool;
auto checkObjectIsNull(const json& j, const std::vector<std::string>& path) -> bool;
auto checkObjectIsNumber(const json& j, const std::vector<std::string>& path) -> bool;
auto checkObjectIsNumberFloat(const json& j, const std::vector<std::string>& path) -> bool;
auto checkObjectIsNumberInteger(const json& j, const std::vector<std::string>& path) -> bool;
auto checkObjectIsNumberUnsigned(const json& j, const std::vector<std::string>& path) -> bool;
auto checkObjectIsObject(const json& j, const std::vector<std::string>& path) -> bool;
auto checkObjectIsPrimitive(const json& j, const std::vector<std::string>& path) -> bool;
auto checkObjectIsString(const json& j, const std::vector<std::string>& path) -> bool;

} // namespace oink_judge::config
