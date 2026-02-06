#pragma once
#include "oink_judge/test_node/verdict_aggregator.hpp"

namespace oink_judge::test_node {

class AggregateCurrent : public VerdictAggregator {
  public:
    AggregateCurrent();

    auto update(std::shared_ptr<VerdictBase> verdict, const std::string& submission_id) -> void override;

    constexpr static auto REGISTERED_NAME = "current";
};

} // namespace oink_judge::test_node
