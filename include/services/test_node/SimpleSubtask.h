#pragma once

#include "services/test_node/Subtask.h"

namespace oink_judge::services::test_node {

class SimpleSubtask : public Subtask {
public:
    SimpleSubtask(const std::string &subtask_id, double score);

    SubtaskVerdict run(const Testset &testset, const std::string &box_id_to_run, const std::string &box_id_to_check) const override;
private:
    double _score;
};

} // namespace oink_judge::services::test_node
