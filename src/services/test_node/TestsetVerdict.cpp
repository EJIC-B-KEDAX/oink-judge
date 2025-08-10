#include "services/test_node/TestsetVerdict.h"

namespace oink_judge::services::test_node {

TestsetVerdict::TestsetVerdict(const Verdict &verdict, const std::vector<SubtaskVerdict> &subtasks_verdicts) : _testset_verdict(verdict), _subtasks_verdicts(subtasks_verdicts) {}

Verdict TestsetVerdict::get_testset_verdict() const {
    return _testset_verdict;
}

const std::vector<SubtaskVerdict> &TestsetVerdict::get_subtasks_verdicts() const {
    return _subtasks_verdicts;
}

} // namespace oink_judge::services::test_node
