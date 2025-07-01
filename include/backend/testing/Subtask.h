#pragma once

#include <string>
#include <vector>
#include "Test.h"
#include "SubtaskVerdict.h"

namespace oink_judge::backend::testing {

class Testset;

class Subtask {
public:
    explicit Subtask(const std::string &subtask_id);
    virtual ~Subtask() = default;

    std::string get_subtask_id() const;
    const std::vector<Test*> &get_tests() const;

    void push_test(Test *test);

    virtual SubtaskVerdict run(const Testset &testset, const std::string &box_id_to_run, const std::string &box_id_to_check) const = 0;

protected:
    std::vector<Test*> &access_tests();

private:
    std::string _subtask_id;

    std::vector<Test*> _tests;
};

} // namespace oink_judge::backend::testing
