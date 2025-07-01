#include "backend/management/ICPCProblem.h"
#include <fstream>
#include "config/Config.h"

namespace oink_judge::backend::management {

using Config = config::Config;
using Verdict = testing::Verdict;
using json = nlohmann::json;

ICPCProblem::ICPCProblem(const std::string &id) : Problem(id) {}

void ICPCProblem::handle_submission(const std::string &submission_id) {
    std::string scripts_dir = Config::instance().get_directory("scripts");
    std::string submission_dir = Config::instance().get_directory("submissions");

    json info;

    std::ifstream config_file(submission_dir + "/" + submission_id + "/info.json");
    if (!config_file.is_open()) {
        throw std::runtime_error("Could not open config file: " + submission_dir + "/" + submission_id + "/info.json");
    }
    config_file >> info;

    std::string username = info["username"];

    int rc = std::system((scripts_dir + "/prepare_for_testing.sh 0 1 " + get_id() + " " + submission_id + " cpp17").c_str());

    if (rc != 0) {
        access_table().set_total_score(username, -42);
        return;
    }

    Verdict verdict;

    for (auto testset : access_config().get_testsets()) {
        verdict = testset->run("0", "1").get_testset_verdict();
    }

    std::system((scripts_dir + "/end_testing.sh 0 1").c_str());

    if (verdict.get_type() == Verdict::Type::OK) {
        access_table().set_total_score(username, 100);
    } else {
        access_table().set_total_score(username, 0);
    }
}

} // namespace oink_judge::backend::management