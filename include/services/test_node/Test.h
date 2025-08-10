#pragma once

#include <string>
#include "services/test_node/Verdict.h"

namespace oink_judge::services::test_node {

class Testset;

class Subtask;

class Test {
public:
    explicit Test(const std::string &test_id);
    virtual ~Test() = default;

    std::string get_test_id() const;

    virtual Verdict run(const Testset &testset, const Subtask &subtask, const std::string &box_id_to_run, const std::string &box_id_to_check) const = 0;

private:
    std::string _test_id;
};

} // namespace oink_judge::services::test_node
