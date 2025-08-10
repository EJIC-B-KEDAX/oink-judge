#pragma once

#include <vector>
#include "services/test_node/SubtaskVerdict.h"

namespace oink_judge::services::test_node {

class TestsetVerdict {
public:
    TestsetVerdict(const Verdict &verdict, const std::vector<SubtaskVerdict> &subtasks_verdicts);

    Verdict get_testset_verdict() const;
    const std::vector<SubtaskVerdict> &get_subtasks_verdicts() const;

private:
    Verdict _testset_verdict;
    std::vector<SubtaskVerdict> _subtasks_verdicts;
};

} // namespace oink_judge::services::test_node
