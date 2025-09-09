#include "services/test_node/DefaultTest.h"
#include "services/test_node/DefaultVerdict.h"
#include "services/test_node/verdict_utils.h"
#include "config/Config.h"
#include <format>
#include <fstream>
#include <iostream>

namespace oink_judge::services::test_node {

using Config = config::Config;

namespace {

[[maybe_unused]] bool registered = []() -> bool {
    TestFactory::instance().register_type(DefaultTest::REGISTERED_NAME,
        [](ProblemBuilder *problem_builder, const std::string &problem_id, const std::string &test_name) -> std::shared_ptr<DefaultTest> {
        return std::make_shared<DefaultTest>(problem_id, test_name);
    });

    return true;
}();

} // namespace

DefaultTest::DefaultTest(const std::string &problem_id, const std::string &name) {
    _name = name;
    _input_path = Config::config().at("directories").at("problems").get<std::string>() + "/" + problem_id + "/tests/" + std::format("{:02}", std::stoi(name));
    _answer_path = Config::config().at("directories").at("problems").get<std::string>() + "/" + problem_id + "/tests/" + std::format("{:02}", std::stoi(name)) + ".a";
    // TODO get it out of problem config
}

std::shared_ptr<Verdict> DefaultTest::run(const std::string &submission_id, const std::vector<std::string> &boxes, json additional_params) {
    std::cout << "Default test testing " << get_name() << std::endl;
    std::string scripts_path = Config::config()["directories"]["scripts"];

    std::string box_id_to_run = boxes[0];
    std::string box_id_to_check = boxes[1];
    double time_limit = additional_params.at("time_limit");
    double memory_limit = additional_params.at("memory_limit");
    double real_time_limit = additional_params.at("real_time_limit");

    int rc = std::system((scripts_path + "/run_in_isolate.sh " + std::to_string(time_limit) + " " + std::to_string(static_cast<int>(memory_limit)) +
        " " + std::to_string(real_time_limit) + " " + _input_path + " " + box_id_to_run).c_str());

    if (rc != 0) {
        return load_verdict_from_meta(get_name(), "meta.txt");
    }

    std::system((scripts_path + "/check.sh " + _answer_path + " " + box_id_to_run + " " + box_id_to_check).c_str());

    return load_verdict_from_checker_output(get_name(), "meta.txt", "checker_err.txt");
}

std::shared_ptr<Verdict> DefaultTest::skip(const std::string &submission_id) {
    auto verdict = std::make_shared<DefaultVerdict>(get_name());

    VerdictType type;
    type.type = VerdictType::SKIPPED;
    type.full_name = "Skipped";
    type.short_name = "SK";

    verdict->set_info({
        type,
        0,
        0,
        0,
        0
    });

    return verdict;
}

size_t DefaultTest::boxes_required() const {
    return 2;
}

const std::string &DefaultTest::get_name() const {
    return _name;
}

} // namespace oink_judge::services::test_node
