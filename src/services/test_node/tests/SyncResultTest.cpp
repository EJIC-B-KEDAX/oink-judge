#include "services/test_node/tests/SyncResultTest.h"
#include "config/problem_config_utils.h"
#include "services/test_node/problem_builders/enable_get_test_by_name.hpp"
#include "services/test_node/ProblemTablesStorage.h"
#include "services/test_node/ProblemBuilder.hpp"
#include "database/TableSubmissions.h"
#include "config/Config.h"
#include <iostream>
#include <fstream>

namespace oink_judge::services::test_node {

using Config = config::Config;
using TableSubmissions = database::TableSubmissions;

namespace {

[[maybe_unused]] bool registered = []() -> bool {
    TestFactory::instance().register_type(SyncResultTest::REGISTERED_NAME,
        [](ProblemBuilder *problem_builder, const std::string &problem_id, const std::string &name) -> std::shared_ptr<Test> {
        return std::make_shared<SyncResultTest>(problem_builder, problem_id, name);
    });

    return true;
}();

} // namespace

SyncResultTest::SyncResultTest(ProblemBuilder *problem_builder, const std::string &problem_id, const std::string &name)
    : _name(name), _problem_id(problem_id) {
    pugi::xml_node testset_config = problem_config::get_test_config(problem_id, name);

    enable_get_test_by_name *problem_builder_with_getter = dynamic_cast<enable_get_test_by_name*>(problem_builder);

    pugi::xml_node test_node = testset_config.child("test");

    std::string test_name = test_node.attribute("name").as_string();
    _test = problem_builder_with_getter->get_test_by_name(test_name);
    if (!_test) {
        throw std::runtime_error("Test not found: " + test_name);
    }
}

std::shared_ptr<Verdict> SyncResultTest::run(const std::string &submission_id, const std::vector<std::string> &boxes, json additional_params) {
    std::cout << "Sync test testing " << get_name() << std::endl;
    if (boxes.size() < boxes_required()) {
        throw std::runtime_error("Not enough boxes provided");
    }

    std::shared_ptr<Verdict> verdict = _test->run(submission_id, boxes, additional_params);

    double score = verdict->get_score();
    std::string verdict_type = verdict->get_type().short_name;
    ProblemTable &table = ProblemTablesStorage::instance().get_table(_problem_id);
    std::string username = TableSubmissions::instance().whose_submission(submission_id);
    std::cout << "Result: " << score << ' ' << verdict_type << std::endl;
    std::cout << verdict->to_json(2).dump(4) << std::endl;

    std::ofstream testing_protocol_file(Config::config().at("directories").at("submissions").get<std::string>() + "/" + submission_id + "/protocol.json");
    testing_protocol_file << verdict->to_json(2).dump(4);
    testing_protocol_file.close();

    TableSubmissions::instance().set_score(submission_id, score);
    TableSubmissions::instance().set_verdict_type(submission_id, verdict_type);
    score = std::max(score, table.get_total_score(username));
    table.set_total_score(username, score);

    return verdict;
}

std::shared_ptr<Verdict> SyncResultTest::skip(const std::string &submission_id) {
    return _test->skip(submission_id);
}

size_t SyncResultTest::boxes_required() const {
    return _test->boxes_required();
}

const std::string &SyncResultTest::get_name() const {
    return _name;
}

} // namespace oink_judge::services::test_node
