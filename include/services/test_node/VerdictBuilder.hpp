#pragma once

#include "DefaultVerdict.h"
#include "ParameterizedTypeFactory.hpp"

namespace oink_judge::services::test_node {

class VerdictBuilder {
public:
    virtual ~VerdictBuilder() = default;

    virtual void clear() = 0;
    virtual void add_verdict(std::shared_ptr<DefaultVerdict> verdict) = 0;
    virtual bool can_score_change() const = 0;
    virtual std::shared_ptr<DefaultVerdict> finalize() = 0;
};

using VerdictBuilderFactory = ParameterizedTypeFactory<std::unique_ptr<VerdictBuilder>, const std::string&>;

} // namespace oink_judge::services::test_node
