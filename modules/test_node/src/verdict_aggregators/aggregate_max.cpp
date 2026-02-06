#include "oink_judge/test_node/verdict_aggregators/aggregate_max.h"

namespace oink_judge::test_node {

namespace {

[[maybe_unused]] const bool REGISTERED = []() -> bool {
    VerdictAggregatorFactory::instance().registerType(AggregateMax::REGISTERED_NAME,
                                                      [](const std::string& params, const std::string& problem_id,
                                                         const std::string& test_name) -> std::unique_ptr<AggregateMax> {
                                                          return std::make_unique<AggregateMax>(problem_id, test_name);
                                                      });

    return true;
}();

} // namespace

AggregateMax::AggregateMax(std::string problem_id, std::string test_name)
    : problem_id_(std::move(problem_id)), test_name_(std::move(test_name)) {}

auto AggregateMax::update(std::shared_ptr<VerdictBase> verdict, const std::string& submission_id) -> void {
    // Implementation of the update logic goes here
}

} // namespace oink_judge::test_node
