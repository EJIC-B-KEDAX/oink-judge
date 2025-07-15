#include "backend/testing/OutputTest.h"
#include "backend/testing/Testset.h"
#include "config/Config.h"

namespace oink_judge::backend::testing {

using Config = config::Config;

OutputTest::OutputTest(const std::string &test_id, const std::string &input_path, const std::string &answer_path) : Test(test_id), _input_path(input_path), _answer_path(answer_path) {}

std::string OutputTest::get_input_path() const {
    return _input_path;
}

std::string OutputTest::get_answer_path() const {
    return _answer_path;
}

Verdict OutputTest::run(const Testset &testset, const Subtask &subtask, const std::string &box_id_to_run, const std::string &box_id_to_check) const {
    std::string scripts_path = Config::config()["directories"]["scripts"];

    int rc = std::system((scripts_path + "/run_in_isolate.sh " + std::to_string(testset.get_time_limit()) + " " + std::to_string(static_cast<int>(testset.get_memory_limit())) +
        " " + std::to_string(testset.get_idle_limit()) + " " + _input_path + " " + box_id_to_run).c_str());

    if (rc != 0) {
        return load_verdict_from_meta("meta.txt");
    }

    std::system((scripts_path + "/check.sh " + _answer_path + " " + box_id_to_run + " " + box_id_to_check).c_str());

    return load_verdict_from_checker_output("meta.txt", "checker_err.txt");
}


} // namespace oink_judge::backend::testing
