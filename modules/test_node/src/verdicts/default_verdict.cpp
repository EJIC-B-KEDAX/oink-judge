#include "oink_judge/test_node/verdicts/default_verdict.h"

namespace oink_judge::test_node {

DefaultVerdict::DefaultVerdict(std::string test_name) : VerdictBase(std::move(test_name)) {}

} // namespace oink_judge::test_node
