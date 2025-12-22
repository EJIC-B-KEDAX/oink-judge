#pragma once
#include "services/test_node/verdicts/DefaultVerdict.h"
#include "ParameterizedTypeFactory.hpp"

namespace oink_judge::services::test_node {

class VerdictAggregator {
public:
    virtual ~VerdictAggregator() = default;
    
    virtual void update(std::shared_ptr<VerdictBase> verdict, const std::string &submission_id) = 0;
};

using VerdictAggregatorFactory = ParameterizedTypeFactory<std::unique_ptr<VerdictAggregator>, const std::string&, const std::string&>;

} // namespace oink_judge::services::test_node
