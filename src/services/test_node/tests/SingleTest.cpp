#include "services/test_node/tests/SingleTest.h"
#include "services/test_node/verdicts/DefaultVerdict.h"
#include "services/test_node/verdict_utils.h"
#include "config/Config.h"
#include <format>
#include <fstream>
#include <iostream>

namespace oink_judge::services::test_node {

using Config = config::Config;

namespace {

[[maybe_unused]] bool registered = []() -> bool {
    TestFactory::instance().register_type(SingleTest::REGISTERED_NAME,
        [](ProblemBuilder *problem_builder, const std::string &problem_id, const std::string &test_name) -> std::shared_ptr<SingleTest> {
        return std::make_shared<SingleTest>(problem_id, test_name);
    });

    return true;
}();

} // namespace

SingleTest::SingleTest(const std::string &problem_id, const std::string &name) {
    _name = name;
    int test_number;
    try {
        test_number = std::stoi(name);
        if (test_number < 0) {
            throw std::invalid_argument("Test name must be a non-negative integer string");
        }
    } catch (const std::exception &e) {
        test_number = 1; // Default to test 01 if conversion fails
    }
    _input_path = Config::config().at("directories").at("problems").get<std::string>() + "/" + problem_id + "/tests/" + std::format("{:02}", test_number);
    _answer_path = Config::config().at("directories").at("problems").get<std::string>() + "/" + problem_id + "/tests/" + std::format("{:02}", test_number) + ".a";
    // TODO get it out of problem config
}

std::shared_ptr<Verdict> SingleTest::run(const std::string &submission_id, const std::vector<std::string> &boxes, json additional_params) {
    std::cout << "Single test testing " << get_name() << std::endl;
    std::string scripts_path = Config::config()["directories"]["scripts"];

    std::string box_id_to_run = boxes[0];
    std::string box_id_to_check = boxes[1];
    double time_limit = additional_params.at("time_limit");
    double memory_limit = additional_params.at("memory_limit");
    double real_time_limit = additional_params.at("real_time_limit");

    int rc = std::system((scripts_path + "/run_in_isolate.sh " + std::to_string(time_limit) + " " + std::to_string(static_cast<int>(memory_limit)) +
        " " + std::to_string(real_time_limit) + " " + _input_path + " " + box_id_to_run).c_str());

    if (rc != 0) {
        return load_verdict_from_meta(get_name(), Config::config().at("testing").at("meta_file").get<std::string>());
    }

    std::system((scripts_path + "/check.sh " + _answer_path + " " + box_id_to_run + " " + box_id_to_check).c_str());

    return load_verdict_from_checker_output(get_name(), Config::config().at("testing").at("meta_file").get<std::string>(), Config::config().at("testing").at("checker_err_file").get<std::string>());
}

std::shared_ptr<Verdict> SingleTest::skip(const std::string &submission_id) {
    auto verdict = std::make_shared<DefaultVerdict>(get_name());

    VerdictType type = VerdictType::SKIPPED;

    verdict->set_info({
        type,
        0,
        0,
        0,
        0
    });

    return verdict;
}

size_t SingleTest::boxes_required() const {
    return 2;
}

const std::string &SingleTest::get_name() const {
    return _name;
}

} // namespace oink_judge::services::test_node
