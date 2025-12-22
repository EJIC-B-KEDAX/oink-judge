#include "services/test_node/problem_builders/DefaultProblemBuilder.h"
#include "config/problem_config_utils.h"

namespace oink_judge::services::test_node {

namespace {

[[maybe_unused]] bool registered = []() -> bool {
    ProblemBuilderFactory::instance().register_type(DefaultProblemBuilder::REGISTERED_NAME,
        [](const std::string &problem_id) -> std::shared_ptr<DefaultProblemBuilder> {
        return std::make_shared<DefaultProblemBuilder>(problem_id);
    });

    return true;
}();

} // namespace

DefaultProblemBuilder::DefaultProblemBuilder(const std::string &problem_id) : _problem_id(problem_id) {}

std::shared_ptr<Test> DefaultProblemBuilder::build() {
    std::vector<std::string> test_names = problem_config::get_all_test_names(_problem_id);

    for (const auto &test_name : test_names) {
        pugi::xml_node test_config = problem_config::get_test_config(_problem_id, test_name);
        std::string test_type = test_config.attribute("type").as_string();
        std::shared_ptr<Test> cur_test = TestFactory::instance().create(test_type, this, _problem_id, test_name);
        add_test(cur_test);
    }

    return get_test_by_name("main");
}

} // namespace oink_judge::services::test_node
