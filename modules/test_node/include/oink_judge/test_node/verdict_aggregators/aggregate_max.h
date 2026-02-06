#pragma once
#include "oink_judge/test_node/verdict_aggregator.hpp"

namespace oink_judge::test_node {

class AggregateMax : public VerdictAggregator {
  public:
    AggregateMax(std::string problem_id, std::string test_name);

    auto update(std::shared_ptr<VerdictBase> verdict, const std::string& submission_id) -> void override;

    constexpr static auto REGISTERED_NAME = "max";

  private:
    std::string problem_id_;
    std::string test_name_;
};

} // namespace oink_judge::test_node
