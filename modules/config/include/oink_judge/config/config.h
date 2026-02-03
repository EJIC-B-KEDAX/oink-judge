#pragma once
#include <chrono>
#include <filesystem>
#include <memory>
#include <nlohmann/json.hpp>
#include <optional>

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
    Config(const std::filesystem::path& config_file_path);

    json config_data_;
};

auto getDirectoryPath(const std::string& key) -> std::optional<std::filesystem::path>;

struct DatabaseConfig {
    std::string host;
    int port;
    std::string username;
    std::string password;
    std::string database_name;
};

auto getDatabaseConfig() -> std::optional<DatabaseConfig>;

auto getServerPort(const std::string& server_name) -> std::optional<int>;

auto getServerHostname(const std::string& server_name) -> std::optional<std::string>;

auto getSessionType(const std::string& session_for) -> std::optional<std::string>;

auto getStartMessage(const std::string& session_for) -> std::optional<std::string>;

auto getTiming(const std::string& timing_name) -> std::optional<std::chrono::duration<double>>;

} // namespace oink_judge::config
