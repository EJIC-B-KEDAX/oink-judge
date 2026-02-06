#include "oink_judge/test_node/verdict_aggregators/aggregate_current.h"

namespace oink_judge::test_node {

namespace {

[[maybe_unused]] const bool REGISTERED = []() -> bool {
    VerdictAggregatorFactory::instance().registerType(
        AggregateCurrent::REGISTERED_NAME,
        [](const std::string& params, const std::string& problem_id,
           const std::string& test_name) -> std::unique_ptr<AggregateCurrent> { return std::make_unique<AggregateCurrent>(); });

    return true;
}();

} // namespace

AggregateCurrent::AggregateCurrent() = default;

auto AggregateCurrent::update(std::shared_ptr<VerdictBase> verdict, const std::string& submission_id) -> void {}

} // namespace oink_judge::test_node
