#pragma once
#include "Testset.h"

namespace oink_judge::backend::testing {

class SimpleTestset : public Testset {
public:
    SimpleTestset(const std::string &testset_id, double time_limit, double memory_limit, double idle_limit);

    TestsetVerdict run(const std::string &box_id_to_run, const std::string &box_id_to_check) const override;
};

} // namespace oink_judge::backend::testing