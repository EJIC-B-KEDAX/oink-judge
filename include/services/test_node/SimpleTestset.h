#pragma once
#include "services/test_node/Testset.h"

namespace oink_judge::services::test_node {

class SimpleTestset : public Testset {
public:
    SimpleTestset(const std::string &testset_id, double time_limit, double memory_limit, double idle_limit);

    TestsetVerdict run(const std::string &box_id_to_run, const std::string &box_id_to_check) const override;
};

} // namespace oink_judge::services::test_node
