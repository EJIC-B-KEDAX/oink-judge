#pragma once

#include <string>
#include <vector>
#include "Subtask.h"
#include "TestsetVerdict.h"

namespace oink_judge::backend::testing {

class Testset {
public:
    explicit Testset(const std::string& testset_id, double time_limit, double memory_limit, double idle_limit);
    virtual ~Testset() = default;

    std::string get_testset_id() const;
    const std::vector<Subtask*> &get_subtasks() const;
    double get_time_limit() const;
    double get_memory_limit() const;
    double get_idle_limit() const;

    void push_subtask(Subtask *subtask);

    virtual TestsetVerdict run(const std::string &box_id_to_run, const std::string &box_id_to_check) const = 0;

protected:
    std::vector<Subtask*> &access_subtasks();

private:
    std::string _testset_id;

    double _time_limit;
    double _memory_limit;
    double _idle_limit;

    std::vector<Subtask*> _subtasks;
};

} // namespace oink_judge::backend::testing