#include "oink_judge/test_node/problem_tables_storage.h"

#include <oink_judge/content_service/content_storage.h>

namespace oink_judge::test_node {

auto ProblemTablesStorage::instance() -> ProblemTablesStorage& {
    static ProblemTablesStorage instance;
    return instance;
}

auto ProblemTablesStorage::getTable(const std::string& problem_id) -> ProblemTable& {
    ensureTableExists(problem_id);
    return tables_.at(problem_id);
}

void ProblemTablesStorage::ensureTableExists(const std::string& problem_id) {
    if (tables_.contains(problem_id)) {
        return;
    }

    tables_.try_emplace(problem_id, problem_id);
}

ProblemTablesStorage::ProblemTablesStorage() = default;

} // namespace oink_judge::test_node
