#pragma once

#include <nlohmann/json.hpp>

namespace oink_judge::config {

using json = nlohmann::json;

const std::string CONFIG_FILE_PATH = CONFIG_DIR;
const std::string CREDENTIAL_FILE_PATH = CREDENTIALS_DIR;

class Config {
public:
    Config(const Config &) = delete;
    Config &operator=(const Config &) = delete;

    static const json &config();
    static const json &credentials();

private:
    Config(const std::string &config_file_path);

    json _config_data;
};

} // namespace oink_judge::config
