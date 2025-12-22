#include "services/test_node/verdict_aggregators/AggregateCurrent.h"

namespace oink_judge::services::test_node {

namespace {

[[maybe_unused]] bool registered = []() {
    VerdictAggregatorFactory::instance().register_type(
        AggregateCurrent::REGISTERED_NAME,
        [](const std::string &params, const std::string &problem_id, const std::string &test_name) -> std::unique_ptr<AggregateCurrent> {
            return std::make_unique<AggregateCurrent>();
        }
    );

    return true;
}();

} // namespace

AggregateCurrent::AggregateCurrent() = default;

void AggregateCurrent::update(std::shared_ptr<VerdictBase> verdict, const std::string &submission_id) {}

} // namespace oink_judge::services::test_node
