#include "services/test_node/ProblemTablesStorage.h"
#include "services/test_node/ProblemBuilder.hpp"
#include "services/data_sender/ContentStorage.h"

namespace oink_judge::services::test_node {

ProblemTablesStorage &ProblemTablesStorage::instance() {
    static ProblemTablesStorage instance;
    return instance;
}

ProblemTable &ProblemTablesStorage::get_table(const std::string &problem_id) {
    ensure_table_exists(problem_id);
    return _tables.at(problem_id);
}

void ProblemTablesStorage::ensure_table_exists(const std::string &problem_id) {
    if (_tables.find(problem_id) != _tables.end()) return;

    _tables.try_emplace(problem_id, problem_id);
}

ProblemTablesStorage::ProblemTablesStorage() = default;

} // namespace oink_judge::services::test_node
