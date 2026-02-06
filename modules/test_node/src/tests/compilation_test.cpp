#include "oink_judge/test_node/tests/compilation_test.h"

#include "oink_judge/test_node/problem_builder.hpp"
#include "oink_judge/test_node/problem_builders/enable_get_test_by_name.hpp"
#include "oink_judge/test_node/verdicts/default_verdict.h"

#include <oink_judge/config/config.h>
#include <oink_judge/config/problem_config_utils.h>
#include <oink_judge/database/table_submissions.h>

namespace oink_judge::test_node {

using Config = config::Config;
using TableSubmissions = database::TableSubmissions;

namespace {

[[maybe_unused]] const bool REGISTERED = []() -> bool {
    TestFactory::instance().registerType(
        CompilationTest::REGISTERED_NAME,
        [](ProblemBuilder* problem_builder, const std::string& problem_id, const std::string& name) -> std::shared_ptr<Test> {
            return std::make_shared<CompilationTest>(problem_builder, problem_id, name);
        });

    return true;
}();

} // namespace

CompilationTest::CompilationTest(ProblemBuilder* problem_builder, std::string problem_id, std::string name)
    : name_(std::move(name)), problem_id_(std::move(problem_id)) {
    pugi::xml_node testset_config = problem_config::getTestConfig(problem_id_, name_).value_or(pugi::xml_node{});

    auto* problem_builder_with_getter = dynamic_cast<EnableGetTestByName*>(problem_builder);

    pugi::xml_node test_node = testset_config.child("test");

    std::string test_name = test_node.attribute("name").as_string();
    test_ = problem_builder_with_getter->getTestByName(test_name);
    if (!test_) {
        throw std::runtime_error("Test not found: " + test_name);
    }
}

auto CompilationTest::run(const std::string& submission_id, const std::vector<std::string>& boxes, json additional_params)
    -> std::shared_ptr<Verdict> {
    if (boxes.size() < boxesRequired()) {
        throw std::runtime_error("Not enough boxes provided");
    }

    std::string scripts_dir = Config::config().at("directories").at("scripts").get<std::string>();
    std::string submissions_dir = Config::config().at("directories").at("submissions").get<std::string>();
    std::string problems_dir = Config::config().at("directories").at("problems").get<std::string>();

    std::string language = TableSubmissions::instance().languageOfSubmission(submission_id);

    std::string compilation_script = scripts_dir + "/compilation/" + language + ".sh"; // TODO change type to fs::path
    std::string output_executale_name = "/var/local/lib/isolate/" + boxes[0] + "/box/solution";
    std::string error_file_name = submissions_dir + "/" + submission_id + "/compilation_error.txt";
    std::string source_file_name = submissions_dir + "/" + submission_id + "/source.cpp";

    int rc_prepare = std::system((Config::config().at("directories").at("scripts").get<std::string>() +
                                  "/prepare_for_testing.sh " + boxes[0] + " " + boxes[1] + " " + problem_id_)
                                     .c_str());

    if (rc_prepare != 0) {
        auto verdict = std::make_shared<DefaultVerdict>(name_);
        DefaultVerdict::VerdictInfo info;
        info.type = VerdictType::FAILED;
        info.score = 0.0;
        info.time_used = 0.0;
        info.memory_used = 0.0;
        info.real_time_used = 0.0;
        verdict->setInfo(info);

        return verdict;
    }

    int rc_compilation =
        std::system((compilation_script + " " + output_executale_name + " " + error_file_name + " " + source_file_name).c_str());

    if (rc_compilation != 0) {
        auto verdict = std::make_shared<DefaultVerdict>(name_);
        DefaultVerdict::VerdictInfo info;
        info.type = VerdictType::COMPILATION_ERROR;
        info.score = 0.0;
        info.time_used = 0.0;
        info.memory_used = 0.0;
        info.real_time_used = 0.0;
        verdict->setInfo(info);

        return verdict;
    }

    int rc_copy_checker = std::system(
        ("cp " + problems_dir + "/" + problem_id_ + "/checker /var/local/lib/isolate/" + boxes[1] + "/box/checker").c_str());

    if (rc_copy_checker != 0) {
        auto verdict = std::make_shared<DefaultVerdict>(name_);
        DefaultVerdict::VerdictInfo info;
        info.type = VerdictType::FAILED;
        info.score = 0.0;
        info.time_used = 0.0;
        info.memory_used = 0.0;
        info.real_time_used = 0.0;
        verdict->setInfo(info);

        return verdict;
    }

    std::shared_ptr<Verdict> result = test_->run(submission_id, boxes, additional_params);

    int rc_clear = std::system(
        (Config::config().at("directories").at("scripts").get<std::string>() + "/end_testing.sh " + boxes[0] + " " + boxes[1])
            .c_str());

    if (rc_clear != 0) {
        auto verdict = std::make_shared<DefaultVerdict>(name_);
        DefaultVerdict::VerdictInfo info;
        info.type = VerdictType::FAILED;
        info.score = 0.0;
        info.time_used = 0.0;
        info.memory_used = 0.0;
        info.real_time_used = 0.0;
        verdict->setInfo(info);

        return verdict;
    }

    return result;
}

auto CompilationTest::skip(const std::string& submission_id) -> std::shared_ptr<Verdict> { return test_->skip(submission_id); }

auto CompilationTest::boxesRequired() const -> size_t { return test_->boxesRequired(); }

auto CompilationTest::getName() const -> const std::string& { return name_; }

} // namespace oink_judge::test_node
