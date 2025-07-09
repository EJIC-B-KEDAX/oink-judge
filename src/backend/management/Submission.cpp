#include "backend/management/Submission.h"
#include "nlohmann/json.hpp"
#include "config/Config.h"
#include <fstream>

namespace oink_judge::backend::management {

using json = nlohmann::json;
using Config = config::Config;

SubmissionInfo load_submission_info(const std::string &submission_id) {
    std::string submission_dir = Config::instance().get_directory("submissions");

    json info;

    std::ifstream config_file(submission_dir + "/" + submission_id + "/info.json");
    if (!config_file.is_open()) {
        throw std::runtime_error("Could not open config file: " + submission_dir + "/" + submission_id + "/info.json");
    }
    config_file >> info;
    config_file.close();

    SubmissionInfo submission_info;
    submission_info.participant = info["participant"];
    submission_info.language = info["language"];
    submission_info.problem_id = info["problem_id"];
    submission_info.sending_time = info["sending_time"];

    return submission_info;
}

void store_submission_info(const std::string &submission_id, const SubmissionInfo &submission_info) {
    std::string submission_dir = Config::instance().get_directory("submissions");

    json info;
    info["participant"] = submission_info.participant;
    info["language"] = submission_info.language;
    info["problem_id"] = submission_info.problem_id;
    info["sending_time"] = submission_info.sending_time;

    std::ofstream config_file(submission_dir + "/" + submission_id + "/info.json");
    if (!config_file.is_open()) {
        throw std::runtime_error("Could not open config file: " + submission_dir + "/" + submission_id + "/info.json");
    }
    config_file << info;
    config_file.close();
}

} // namespace oink_judge::backend::management