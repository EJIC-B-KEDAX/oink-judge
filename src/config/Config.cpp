#include "config/Config.h"
#include <fstream>
#include <iostream>

namespace oink_judge::config {

using json = nlohmann::json;

const json &Config::config() {
    static Config config(CONFIG_FILE_PATH);
    return config._config_data;
}

const json &Config::credentials() {
    static Config credentials(CREDENTIAL_FILE_PATH);
    return credentials._config_data;
}

Config::Config(const std::string &config_file_path) {
    std::ifstream config_file(config_file_path);
    if (!config_file.is_open()) {
        throw std::runtime_error("Could not open config file: " + config_file_path);
    }
    config_file >> _config_data;
}

} // namespace oink_judge::config