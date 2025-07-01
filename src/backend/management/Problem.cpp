#include "backend/management/Problem.h"
#include <utility>

namespace oink_judge::backend::management {

Problem::Problem(const std::string &id) : _id(id), _config(_id), _table(_config) {}

std::string Problem::get_id() const {
    return _id;
}

std::string Problem::get_short_name() const {
    return get_id(); // TODO: add loading info out of problem.xml
}

double Problem::get_ok_score() const {
    return _ok_score;
}

const ProblemTable &Problem::get_table() const {
    return _table;
}

const ProblemConfig &Problem::get_config() const {
    return _config;
}

double Problem::get_participant_score(const std::string &username) const {
    return _table.get_total_score(username);
}

ProblemTable &Problem::access_table() {
    return _table;
}

ProblemConfig &Problem::access_config() {
    return _config;
}

void Problem::set_ok_score(double score) {
    _ok_score = score;
}

} // namespace oink_judge::backend::management