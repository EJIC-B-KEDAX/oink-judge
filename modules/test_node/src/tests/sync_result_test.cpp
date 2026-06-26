#include "oink_judge/test_node/tests/sync_result_test.h"

#include "oink_judge/test_node/problem_builder.hpp"
#include "oink_judge/test_node/problem_builders/enable_get_test_by_name.hpp"
// #include "oink_judge/test_node/problem_tables_storage.h"

#include <oink_judge/config/common_utils.h>
#include <oink_judge/config/problem_config_utils.h>
#include <oink_judge/database/table_submissions.h>
#include <oink_judge/logger/logger.h>

#include <fstream>

namespace oink_judge::test_node {

using config::requireHasValue;
using database::TableSubmissions;

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

auto SyncResultTest::run(std::string submission_id, std::vector<std::string> boxes, json additional_params)
    -> awaitable<std::shared_ptr<Verdict>> {
    if (boxes.size() < boxesRequired()) {
        throw std::runtime_error("Not enough boxes provided");
    }

    std::shared_ptr<Verdict> verdict = co_await test_->run(submission_id, boxes, additional_params);

    double score = verdict->getScore();
    std::string verdict_type = verdict->getType().short_name;
    // ProblemTable& table = ProblemTablesStorage::instance().getTable(problem_id_);
    // std::string username = co_await TableSubmissions::instance().whoseSubmission(submission_id);

    std::ofstream testing_protocol_file(requireHasValue(config::getDirectoryPath("submissions")) / submission_id /
                                        "protocol.json");
    testing_protocol_file << verdict->toJson(2).dump(4);
    testing_protocol_file.close();

    co_await TableSubmissions::instance().setScore(submission_id, score);
    co_await TableSubmissions::instance().setVerdictType(submission_id, verdict_type);
    // score = std::max(score, table.getTotalScore(username));
    // table.setTotalScore(username, score);

    co_return verdict;
}

auto SyncResultTest::skip(std::string submission_id) -> awaitable<std::shared_ptr<Verdict>> { return test_->skip(submission_id); }

auto SyncResultTest::boxesRequired() const -> size_t { return test_->boxesRequired(); }

auto SyncResultTest::getName() const -> const std::string& { return name_; }

auto registerSyncResultTestType() -> void {
    TestFactory::instance().registerType(
        SyncResultTest::REGISTERED_NAME,
        [](ProblemBuilder* problem_builder, const std::string& problem_id, const std::string& name) -> std::shared_ptr<Test> {
            return std::make_shared<SyncResultTest>(problem_builder, problem_id, name);
        });
}

} // namespace oink_judge::test_node
