#pragma once

#include <vector>
#include "Verdict.h"

namespace oink_judge::backend::testing {

class SubtaskVerdict {
public:
    SubtaskVerdict(const Verdict &verdict, const std::vector<Verdict> &tests_verdicts);

    Verdict get_subtask_verdict() const;
    const std::vector<Verdict> &get_tests_verdicts() const;
private:
    Verdict _subtask_verdict;
    std::vector<Verdict> _tests_verdicts;
};

} // namespace oink_judge::backend::testing
