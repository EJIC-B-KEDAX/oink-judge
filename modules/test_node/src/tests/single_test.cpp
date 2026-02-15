#include "oink_judge/test_node/tests/single_test.h"

#include "oink_judge/test_node/test_node_config_utils.h"
#include "oink_judge/test_node/verdict_utils.h"
#include "oink_judge/test_node/verdicts/default_verdict.h"

#include <format>
#include <oink_judge/config/config.h>
#include <oink_judge/logger/logger.h>

namespace oink_judge::test_node {

using logger::requireHasValue;

namespace {

[[maybe_unused]] const bool REGISTERED = []() -> bool {
    TestFactory::instance().registerType(SingleTest::REGISTERED_NAME,
                                         [](ProblemBuilder* problem_builder, const std::string& problem_id,
                                            const std::string& test_name) -> std::shared_ptr<SingleTest> {
                                             return std::make_shared<SingleTest>(problem_id, test_name);
                                         });

    return true;
}();

} // namespace

SingleTest::SingleTest(const std::string& problem_id, std::string name) : name_(std::move(name)) {
    int test_number = 0;
    try {
        test_number = std::stoi(name);
        if (test_number < 0) {
            throw std::invalid_argument("Test name must be a non-negative integer string");
        }
    } catch (const std::exception& e) {
        test_number = 1; // Default to test 01 if conversion fails
    }

    fs::path problems_dir = requireHasValue(config::getDirectoryPath("problems"));

    input_path_ = problems_dir / problem_id / "tests" / std::format("{:02}", test_number);
    answer_path_ = problems_dir / problem_id / "tests" / (std::format("{:02}", test_number) + ".a");
    // TODO get it out of problem config
}

auto SingleTest::run(const std::string& submission_id, const std::vector<std::string>& boxes, json additional_params)
    -> std::shared_ptr<Verdict> {
    fs::path scripts_path = requireHasValue(config::getDirectoryPath("scripts"));

    const std::string& box_id_to_run = boxes[0];
    const std::string& box_id_to_check = boxes[1];
    double time_limit = additional_params.at("time_limit");
    double memory_limit = additional_params.at("memory_limit");
    double real_time_limit = additional_params.at("real_time_limit");

    int rc = std::system(((scripts_path / "run_in_isolate.sh").string() + " " + std::to_string(time_limit) + " " +
                          std::to_string(static_cast<int>(memory_limit)) + " " + std::to_string(real_time_limit) + " " +
                          input_path_.string() + " " + box_id_to_run)
                             .c_str());

    if (rc != 0) {
        return loadVerdictFromMeta(getName(), requireHasValue(getTestingLogFilePath("meta_file")));
    }

    std::system(((scripts_path / "check.sh").string() + " " + answer_path_.string() + " " + box_id_to_run + " " + box_id_to_check)
                    .c_str());

    return loadVerdictFromCheckerOutput(getName(), requireHasValue(getTestingLogFilePath("meta_file")),
                                        requireHasValue(getTestingLogFilePath("checker_err_file")));
}

auto SingleTest::skip(const std::string& submission_id) -> std::shared_ptr<Verdict> {
    auto verdict = std::make_shared<DefaultVerdict>(getName());
    VerdictType type = VerdictType::SKIPPED;

    verdict->setInfo({.type = type, .score = 0, .time_used = 0, .memory_used = 0, .real_time_used = 0});

    return verdict;
}

auto SingleTest::boxesRequired() const -> size_t { return 2; }

auto SingleTest::getName() const -> const std::string& { return name_; }

} // namespace oink_judge::test_node
