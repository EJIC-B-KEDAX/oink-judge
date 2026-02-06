#pragma once
#include "oink_judge/test_node/test.hpp"

#include <string>

namespace oink_judge::test_node {
class ProblemTable {
  public:
    explicit ProblemTable(std::string problem_id);

    auto checkTableActuality() -> bool;

    [[nodiscard]] auto getTableName() const -> std::string;
    [[nodiscard]] auto getRawTableName() const -> std::string;
    [[nodiscard]] auto getColumns() const -> std::vector<std::string>;

    auto setTotalScore(const std::string& username, double total_score) -> void;
    auto setTestedOnRevision(const std::string& username, int revision) -> void;
    auto setScoreOnTest(const std::string& username, const std::shared_ptr<Test>& test, double score) -> void;
    auto addNewUser(const std::string& username) -> void;

    [[nodiscard]] auto getTotalScore(const std::string& username) const -> double;
    [[nodiscard]] auto getTestedOnRevision(const std::string& username) const -> int;
    [[nodiscard]] auto getScoreOnTest(const std::string& username, const std::shared_ptr<Test>& test) const -> double;
    [[nodiscard]] auto doesUserHaveSubmission(const std::string& username) const -> bool;

  private:
    [[nodiscard]] auto getRightColumns() const -> std::vector<std::string>;
    auto touchTable() -> void;

    std::string id_;
};

} // namespace oink_judge::test_node
