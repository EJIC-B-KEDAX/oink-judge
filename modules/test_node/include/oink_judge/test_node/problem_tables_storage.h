#pragma once
#include "oink_judge/test_node/problem_table.h"

namespace oink_judge::test_node {

class ProblemTablesStorage {
  public:
    static auto instance() -> ProblemTablesStorage&;

    ProblemTablesStorage(const ProblemTablesStorage&) = delete;
    auto operator=(const ProblemTablesStorage&) -> ProblemTablesStorage& = delete;
    ProblemTablesStorage(ProblemTablesStorage&&) = delete;
    auto operator=(ProblemTablesStorage&&) -> ProblemTablesStorage& = delete;
    ~ProblemTablesStorage() = default;

    [[nodiscard]] auto getTable(const std::string& problem_id) -> ProblemTable&;

  private:
    void ensureTableExists(const std::string& problem_id);

    ProblemTablesStorage();

    std::map<std::string, ProblemTable> tables_;
};

} // namespace oink_judge::test_node
