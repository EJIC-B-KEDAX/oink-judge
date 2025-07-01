#include "backend/testing/SimpleTestset.h"
#include <cassert>

namespace oink_judge::backend::testing {

SimpleTestset::SimpleTestset(const std::string &testset_id, double time_limit, double memory_limit, double idle_limit) : Testset(testset_id, time_limit, memory_limit, idle_limit) {}


TestsetVerdict SimpleTestset::run(const std::string &box_id_to_run, const std::string &box_id_to_check) const {
    assert(!get_subtasks().empty());

    std::vector<SubtaskVerdict> verdicts;

    for (Subtask *subtask : get_subtasks()) {
        verdicts.push_back(subtask->run(*this, box_id_to_run, box_id_to_check));
    }

    Verdict testset_verdict = verdicts[0].get_subtask_verdict();
    for (int i = 1; i < verdicts.size(); i++) {
        testset_verdict = testset_verdict + verdicts[i].get_subtask_verdict();
    }

    return {testset_verdict, verdicts};
}

} // namespace oink_judge::backend::testing
