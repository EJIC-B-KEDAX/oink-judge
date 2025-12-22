#include "services/test_node/verdict_aggregators/AggregateMax.h"

namespace oink_judge::services::test_node {

namespace {

[[maybe_unused]] bool registered = []() {
    VerdictAggregatorFactory::instance().register_type(
        AggregateMax::REGISTERED_NAME,
        [](const std::string &params, const std::string &problem_id, const std::string &test_name) -> std::unique_ptr<AggregateMax> {
            return std::make_unique<AggregateMax>(problem_id, test_name);
        }
    );

    return true;
}();

} // namespace

AggregateMax::AggregateMax(const std::string &problem_id, const std::string &test_name)
    : _problem_id(problem_id), _test_name(test_name) {}

void AggregateMax::update(std::shared_ptr<VerdictBase> verdict, const std::string &submission_id) {
    // Implementation of the update logic goes here

}

} // namespace oink_judge::services::test_node
