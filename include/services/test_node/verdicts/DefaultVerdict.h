#pragma once
#include "VerdictBase.h"
#include "enable_get_problem_verdict.hpp"

namespace oink_judge::services::test_node {

class DefaultVerdict : public VerdictBase, public enable_get_problem_verdict<VerdictBase> {
public:
    explicit DefaultVerdict(const std::string &test_name);
};

} // namespace oink_judge::services::test_node