#pragma once
#include "oink_judge/factory/parameterized_type_factory.hpp"

#include <oink_judge/test_node/verdicts/default_verdict.h>

namespace oink_judge::test_node {

class VerdictAggregator {
  public:
    VerdictAggregator(const VerdictAggregator&) = delete;
    auto operator=(const VerdictAggregator&) -> VerdictAggregator& = delete;
    VerdictAggregator(VerdictAggregator&&) = delete;
    auto operator=(VerdictAggregator&&) -> VerdictAggregator& = delete;
    virtual ~VerdictAggregator() = default;

    virtual auto update(std::shared_ptr<VerdictBase> verdict, const std::string& submission_id) -> void = 0;

  protected:
    VerdictAggregator() = default;
};

using VerdictAggregatorFactory =
    factory::ParameterizedTypeFactory<std::unique_ptr<VerdictAggregator>, const std::string&, const std::string&>;

} // namespace oink_judge::test_node
