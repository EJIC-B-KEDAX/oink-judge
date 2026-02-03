#include "oink_judge/config/config.h"

#include <fstream>

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
    // Reset existing instances and reload from the configured file paths.
    if (config_instance) {
        config_instance.reset();
    }
    if (credentials_instance) {
        credentials_instance.reset();
    }

    config_instance = std::unique_ptr<Config>(new Config(config_file_path));
    credentials_instance = std::unique_ptr<Config>(new Config(credentials_file_path));
}

Config::Config(const std::filesystem::path& config_file_path) {
    std::ifstream config_file(config_file_path);
    if (!config_file.is_open()) {
        throw std::runtime_error("Could not open config file: " + config_file_path.string());
    }
    config_file >> config_data_;
}

auto getDirectoryPath(const std::string& key) -> std::optional<std::filesystem::path> {
    const auto& config_data = Config::config();
    if (!config_data.contains("directories") || !config_data["directories"].contains(key) ||
        !config_data["directories"][key].is_string()) {
        return std::nullopt;
    }

    return std::filesystem::path(config_data["directories"][key].get<std::string>());
}

auto getDatabaseConfig() -> std::optional<DatabaseConfig> {
    const auto& config_data = Config::config();
    const auto& credentials_data = Config::credentials();

    if (!credentials_data.contains("database") || !credentials_data["database"].contains("password") ||
        !credentials_data["database"]["password"].is_string()) {

        return std::nullopt;
    }
    if (!config_data.contains("database") || !config_data["database"].contains("host") ||
        !config_data["database"]["host"].is_string() || !config_data["database"].contains("port") ||
        !config_data["database"]["port"].is_number_integer() || !config_data["database"].contains("username") ||
        !config_data["database"]["username"].is_string() || !config_data["database"].contains("dbname") ||
        !config_data["database"]["dbname"].is_string()) {

        return std::nullopt;
    }

    const auto& db_config = config_data["database"];
    DatabaseConfig result;
    result.host = db_config["host"].get<std::string>();
    result.port = db_config["port"].get<int>();
    result.username = db_config["username"].get<std::string>();
    result.password = credentials_data["database"]["password"].get<std::string>();
    result.database_name = db_config["dbname"].get<std::string>();

    return result;
}

auto getServerPort(const std::string& server_name) -> std::optional<int> {
    const auto& config_data = Config::config();
    if (!config_data.contains("ports") || !config_data["ports"].contains(server_name) ||
        !config_data["ports"][server_name].is_number_integer()) {
        return std::nullopt;
    }

    return config_data["ports"][server_name].get<int>();
}

auto getServerHostname(const std::string& server_name) -> std::optional<std::string> {
    const auto& config_data = Config::config();
    if (!config_data.contains("hosts") || !config_data["hosts"].contains(server_name) ||
        !config_data["hosts"][server_name].is_string()) {
        return std::nullopt;
    }

    return config_data["hosts"][server_name].get<std::string>();
}

auto getSessionType(const std::string& session_for) -> std::optional<std::string> {
    const auto& config_data = Config::config();
    if (!config_data.contains("sessions") || !config_data["sessions"].contains(session_for) ||
        !config_data["sessions"][session_for].contains("type") || !config_data["sessions"][session_for]["type"].is_string()) {
        return std::nullopt;
    }

    return config_data["sessions"][session_for]["type"].get<std::string>();
}

auto getStartMessage(const std::string& session_for) -> std::optional<std::string> {
    const auto& config_data = Config::config();
    if (!config_data.contains("start_messages") || !config_data["start_messages"].contains(session_for) ||
        !config_data["start_messages"][session_for].is_string()) {
        return std::nullopt;
    }

    return config_data["start_messages"][session_for].get<std::string>();
}

auto getTiming(const std::string& timing_name) -> std::optional<std::chrono::duration<double>> {
    const auto& config_data = Config::config();
    if (!config_data.contains("timings") || !config_data["timings"].contains(timing_name) ||
        !config_data["timings"][timing_name].is_number()) {
        return std::nullopt;
    }

    double seconds = config_data["timings"][timing_name].get<double>();
    return std::chrono::duration<double>(seconds);
}

} // namespace oink_judge::config