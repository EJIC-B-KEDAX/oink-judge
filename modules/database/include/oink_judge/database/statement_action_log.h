#pragma once
#include <cstddef>
#include <cstdint>
#include <string>
#include <unordered_set>
#include <vector>

namespace oink_judge::database {

enum class StatementActionType : std::uint8_t { PREPARE, UNPREPARE };

struct StatementAction {
    StatementActionType type;
    std::string name;
    std::string sql;
};

class StatementActionLog {
  public:
    auto appendPrepare(std::string name, std::string sql) -> std::size_t;
    auto appendUnprepare(std::string name) -> std::size_t;
    [[nodiscard]] auto actions() const -> const std::vector<StatementAction>&;

  private:
    std::vector<StatementAction> actions_;
    std::unordered_set<std::string> prepared_names_;
};

} // namespace oink_judge::database
