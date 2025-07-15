#pragma once

#include <nlohmann/json.hpp>

namespace oink_judge::config {

using json = nlohmann::json;

const std::string CONFIG_FILE_PATH = "../config.json";
const std::string CREDENTIAL_FILE_PATH = "../credentials.json";

class Config {
public:
    Config(const Config &) = delete;
    Config &operator=(const Config &) = delete;

    static Config &config();
    static Config &credentials();

    const json &operator[](const std::string &key) const;

private:
    Config(const std::string &config_file_path);

    json _config_data;
};

} // namespace oink_judge::config
