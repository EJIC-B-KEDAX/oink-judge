#pragma once
#include "services/test_node/VerdictAggregator.hpp"

namespace oink_judge::services::test_node {

class AggregateCurrent : public VerdictAggregator {
public:
    AggregateCurrent();

    void update(std::shared_ptr<VerdictBase> verdict, const std::string &submission_id) override;

    constexpr static auto REGISTERED_NAME = "current";
};

} // namespace oink_judge::services::test_node
