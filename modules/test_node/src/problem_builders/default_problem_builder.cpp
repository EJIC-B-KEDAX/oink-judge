#include "oink_judge/test_node/problem_builders/default_problem_builder.h"

#include <oink_judge/config/problem_config_utils.h>

namespace oink_judge::test_node {

namespace {

[[maybe_unused]] const bool REGISTERED = []() -> bool {
    ProblemBuilderFactory::instance().registerType(DefaultProblemBuilder::REGISTERED_NAME,
                                                   [](const std::string& problem_id) -> std::shared_ptr<DefaultProblemBuilder> {
                                                       return std::make_shared<DefaultProblemBuilder>(problem_id);
                                                   });

    return true;
}();

} // namespace

DefaultProblemBuilder::DefaultProblemBuilder(std::string problem_id) : problem_id_(std::move(problem_id)) {}

auto DefaultProblemBuilder::build() -> std::shared_ptr<Test> {
    std::vector<std::string> test_names = problem_config::getAllTestNames(problem_id_).value_or(std::vector<std::string>{});

    for (const auto& test_name : test_names) {
        pugi::xml_node test_config = problem_config::getTestConfig(problem_id_, test_name).value_or(pugi::xml_node{});
        std::string test_type = test_config.attribute("type").as_string();
        std::shared_ptr<Test> cur_test = TestFactory::instance().create(test_type, this, problem_id_, test_name); // NOLINT
        addTest(cur_test);
    }

    return getTestByName("main");
}

} // namespace oink_judge::test_node
