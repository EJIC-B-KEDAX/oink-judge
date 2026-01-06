#pragma once

#include <nlohmann/json.hpp>

namespace oink_judge::config {

using json = nlohmann::json;


class Config {
public:
    Config(const Config &) = delete;
    Config &operator=(const Config &) = delete;

    static const json &config();
    static const json &credentials();

    static void set_config_file_path(const std::string &path);
    static void set_credential_file_path(const std::string &path);

private:
    static std::string CONFIG_FILE_PATH; // Path to the main configuration file, must be set before use
    static std::string CREDENTIAL_FILE_PATH; // Path to the main credentials file, must be set before use

    Config(const std::string &config_file_path);

    json _config_data;
};

} // namespace oink_judge::config
