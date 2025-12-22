#pragma once
#include "ProblemTable.h"

namespace oink_judge::services::test_node {

class ProblemTablesStorage {
public:
    static ProblemTablesStorage &instance();

    ProblemTablesStorage(const ProblemTablesStorage &) = delete;
    ProblemTablesStorage &operator=(const ProblemTablesStorage &) = delete;

    ProblemTable &get_table(const std::string &problem_id);

private:
    void ensure_table_exists(const std::string &problem_id);

    ProblemTablesStorage();

    std::map<std::string, ProblemTable> _tables;
};

} // namespace oink_judge::services::test_node
