#include "backend/testing/SubtaskVerdict.h"

namespace oink_judge::backend::testing {

SubtaskVerdict::SubtaskVerdict(const Verdict &verdict, const std::vector<Verdict> &tests_verdicts) : _subtask_verdict(verdict), _tests_verdicts(tests_verdicts) {}


Verdict SubtaskVerdict::get_subtask_verdict() const {
    return _subtask_verdict;
}

const std::vector<Verdict> &SubtaskVerdict::get_tests_verdicts() const {
    return _tests_verdicts;
}

} // namespace oink_judge::backend::testing
