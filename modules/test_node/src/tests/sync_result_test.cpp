#include "oink_judge/test_node/tests/sync_result_test.h"

#include "oink_judge/test_node/problem_builder.hpp"
#include "oink_judge/test_node/problem_builders/enable_get_test_by_name.hpp"
#include "oink_judge/test_node/problem_tables_storage.h"

#include <fstream>
#include <oink_judge/config/config.h>
#include <oink_judge/config/problem_config_utils.h>
#include <oink_judge/database/table_submissions.h>

namespace oink_judge::test_node {

using Config = config::Config;
using TableSubmissions = database::TableSubmissions;

namespace {

[[maybe_unused]] const bool REGISTERED = []() -> bool {
    TestFactory::instance().registerType(
        SyncResultTest::REGISTERED_NAME,
        [](ProblemBuilder* problem_builder, const std::string& problem_id, const std::string& name) -> std::shared_ptr<Test> {
            return std::make_shared<SyncResultTest>(problem_builder, problem_id, name);
        });

    return true;
}();

} // namespace

SyncResultTest::SyncResultTest(ProblemBuilder* problem_builder, std::string problem_id, std::string name)
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

auto SyncResultTest::run(const std::string& submission_id, const std::vector<std::string>& boxes, json additional_params)
    -> std::shared_ptr<Verdict> {
    if (boxes.size() < boxesRequired()) {
        throw std::runtime_error("Not enough boxes provided");
    }

    std::shared_ptr<Verdict> verdict = test_->run(submission_id, boxes, additional_params);

    double score = verdict->getScore();
    std::string verdict_type = verdict->getType().short_name;
    ProblemTable& table = ProblemTablesStorage::instance().getTable(problem_id_);
    std::string username = TableSubmissions::instance().whoseSubmission(submission_id);

    std::ofstream testing_protocol_file(Config::config().at("directories").at("submissions").get<std::string>() + "/" +
                                        submission_id + "/protocol.json");
    testing_protocol_file << verdict->toJson(2).dump(4);
    testing_protocol_file.close();

    TableSubmissions::instance().setScore(submission_id, score);
    TableSubmissions::instance().setVerdictType(submission_id, verdict_type);
    score = std::max(score, table.getTotalScore(username));
    table.setTotalScore(username, score);

    return verdict;
}

auto SyncResultTest::skip(const std::string& submission_id) -> std::shared_ptr<Verdict> { return test_->skip(submission_id); }

auto SyncResultTest::boxesRequired() const -> size_t { return test_->boxesRequired(); }

auto SyncResultTest::getName() const -> const std::string& { return name_; }

} // namespace oink_judge::test_node
