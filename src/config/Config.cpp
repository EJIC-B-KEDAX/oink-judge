#include "config/Config.h"
#include <fstream>
#include <iostream>

namespace oink_judge::config {

using json = nlohmann::json;

Config &Config::config() {
    static Config config(CONFIG_FILE_PATH);
    return config;
}

Config &Config::credentials() {
    static Config credentials(CREDENTIAL_FILE_PATH);
    return credentials;
}

const json &Config::operator[](const std::string &key) const {
    return _config_data.at(key);
}



Config::Config(const std::string &config_file_path) {
    std::ifstream config_file(config_file_path);
    if (!config_file.is_open()) {
        throw std::runtime_error("Could not open config file: " + config_file_path);
    }
    config_file >> _config_data;
}

} // namespace oink_judge::config