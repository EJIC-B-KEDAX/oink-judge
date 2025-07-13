#include "config/Config.h"
#include <fstream>
#include <iostream>

namespace oink_judge::config {

using json = nlohmann::json;

Config &Config::instance() {
    static Config instance;
    return instance;
}

int Config::get_port(const std::string &key) const {
    if (_config_data["ports"].contains(key) && _config_data["ports"][key].is_number_integer()) {
        return _config_data["ports"][key].get<int>();
    }

    throw std::runtime_error("Key not found or not an integer: " + key);
}

std::string Config::get_directory(const std::string &key) const {
    if (_config_data["directories"].contains(key) && _config_data["directories"][key].is_string()) {
        return _config_data["directories"][key].get<std::string>();
    }

    throw std::runtime_error("Key not found or not a string: " + key);
}

json Config::get_bound(const std::string &key) const {
    if (_config_data["bounds"].contains(key)) {
        return _config_data["bounds"][key];
    }

    throw std::runtime_error("Key not found: " + key);
}

Config::Config() {
    std::ifstream config_file(CONFIG_FILE_PATH);
    if (!config_file.is_open()) {
        throw std::runtime_error("Could not open config file: " + CONFIG_FILE_PATH);
    }
    config_file >> _config_data;
}

} // namespace oink_judge::config