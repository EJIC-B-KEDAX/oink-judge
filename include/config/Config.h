#pragma once

#include <nlohmann/json.hpp>

namespace oink_judge::config {

using json = nlohmann::json;

const std::string CONFIG_FILE_PATH = "../config.json";

class Config {
public:
    Config(const Config &) = delete;
    Config &operator=(const Config &) = delete;

    static Config &instance();

    int get_port(const std::string &key) const;

    std::string get_directory(const std::string &key) const;

    json get_bound(const std::string &key) const;

private:
    Config();

    json _config_data;
};

} // namespace oink_judge::config
