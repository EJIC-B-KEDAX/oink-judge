#include "services/test_node/verdicts/DefaultVerdict.h"

namespace oink_judge::services::test_node {

DefaultVerdict::DefaultVerdict(const std::string &test_name)
    : VerdictBase(test_name), enable_get_problem_verdict<VerdictBase>() {}

} // namespace oink_judge::services::test_node
