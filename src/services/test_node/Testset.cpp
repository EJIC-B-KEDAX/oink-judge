#include "services/test_node/Testset.h"

namespace oink_judge::services::test_node {

Testset::Testset(const std::string &testset_id, double time_limit, double memory_limit, double idle_limit) : _testset_id(testset_id),
    _time_limit(time_limit), _memory_limit(memory_limit), _idle_limit(idle_limit) {}

std::string Testset::get_testset_id() const {
    return _testset_id;
}

const std::vector<Subtask*> &Testset::get_subtasks() const {
    return _subtasks;
}

double Testset::get_time_limit() const {
    return _time_limit;
}

double Testset::get_memory_limit() const {
    return _memory_limit;
}

double Testset::get_idle_limit() const {
    return _idle_limit;
}

void Testset::push_subtask(Subtask *subtask) {
    _subtasks.push_back(subtask);
}

std::vector<Subtask*> &Testset::access_subtasks() {
    return _subtasks;
}

} // namespace oink_judge::services::test_node
