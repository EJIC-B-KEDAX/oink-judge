#pragma once
#include "services/test_node/VerdictAggregator.hpp"

namespace oink_judge::services::test_node {

class AggregateMax : public VerdictAggregator {
public:
    AggregateMax(const std::string &problem_id, const std::string &test_name);

    void update(std::shared_ptr<VerdictBase> verdict, const std::string &submission_id) override;

    constexpr static auto REGISTERED_NAME = "max";

private:
    std::string _problem_id;
    std::string _test_name;
};

} // namespace oink_judge::services::test_node
