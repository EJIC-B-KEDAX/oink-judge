#include "oink_judge/test_node/tests/compilation_test.h"

#include "oink_judge/test_node/problem_builder.hpp"
#include "oink_judge/test_node/problem_builders/enable_get_test_by_name.hpp"
#include "oink_judge/test_node/verdicts/default_verdict.h"

#include <oink_judge/config/config.h>
#include <oink_judge/config/problem_config_utils.h>
#include <oink_judge/database/table_submissions.h>
#include <oink_judge/logger/logger.h>

namespace oink_judge::test_node {

namespace fs = std::filesystem;

using database::TableSubmissions;
using logger::requireHasValue;

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

    fs::path scripts_dir = requireHasValue(config::getDirectoryPath("scripts"));
    fs::path submissions_dir = requireHasValue(config::getDirectoryPath("submissions"));
    fs::path problems_dir = requireHasValue(config::getDirectoryPath("problems"));

    std::string language = TableSubmissions::instance().languageOfSubmission(submission_id);

    fs::path compilation_script = scripts_dir / "compilation" / (language + ".sh"); // TODO change type to fs::path
    fs::path output_executale_name = fs::path("/var/local/lib/isolate") / boxes[0] / "box/solution";
    fs::path error_file_name = submissions_dir / submission_id / "compilation_error.txt";
    fs::path source_file_name = submissions_dir / submission_id / "source.cpp";

    int rc_prepare = std::system(
        ((scripts_dir / "prepare_for_testing.sh").string() + " " + boxes[0] + " " + boxes[1] + " " + problem_id_).c_str());

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

    int rc_compilation = std::system((compilation_script.string() + " " + output_executale_name.string() + " " +
                                      error_file_name.string() + " " + source_file_name.string())
                                         .c_str());

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

    int rc_copy_checker = std::system(("cp " + (problems_dir / problem_id_ / "checker").string() + " " +
                                       (fs::path("/var/local/lib/isolate") / boxes[1] / "box/checker").string())
                                          .c_str());

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

    int rc_clear = std::system(((scripts_dir / "end_testing.sh").string() + " " + boxes[0] + " " + boxes[1]).c_str());

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
