#include "services/test_node/SimpleSubtask.h"
#include <cassert>

namespace oink_judge::services::test_node {

SimpleSubtask::SimpleSubtask(const std::string &subtask_id, double score) : Subtask(subtask_id), _score(score) {}

SubtaskVerdict SimpleSubtask::run(const Testset &testset, const std::string &box_id_to_run, const std::string &box_id_to_check) const {
    assert(!get_tests().empty());

    std::vector<Verdict> verdicts;

    for (Test *test : get_tests()) {
        verdicts.push_back(test->run(testset, *this, box_id_to_run, box_id_to_check));
    }

    Verdict subtask_verdict = verdicts[0];
    for (int i = 1; i < verdicts.size(); i++) {
        subtask_verdict = subtask_verdict + verdicts[i];
    }

    subtask_verdict.set_score(_score * subtask_verdict.get_score() / 100.0);

    subtask_verdict.set_score_merge_policy(Verdict::ScoreMergePolicy::Sum);

    return {subtask_verdict, verdicts};
}

} // namespace oink_judge::services::test_node
