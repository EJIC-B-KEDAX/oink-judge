#include "backend/management/Problem.h"
#include <utility>

namespace oink_judge::backend::management {

Problem::Problem(std::string id) : _id(std::move(id)) {}

const std::string &Problem::get_id() const {
    return _id;
}

const std::string &Problem::get_short_name() const {
    return get_id(); // TODO: add loading info out of problem.xml
}

double Problem::get_ok_score() const {
    return _ok_score;
}

void Problem::set_ok_score(double score) {
    _ok_score = score;
}

} // namespace oink_judge::backend::management