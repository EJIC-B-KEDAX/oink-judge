#pragma once
#include "oink_judge/database/statements.h"

#include <cstddef>
#include <cstdint>
#include <string>
#include <unordered_map>
#include <vector>

namespace oink_judge::database {

enum class StatementActionType : std::uint8_t { PREPARE, UNPREPARE };

struct StatementAction {
    StatementActionType type = StatementActionType::PREPARE;
    StatementsBlock block;
};

class StatementActionLog {
  public:
    auto appendPrepare(StatementsBlock block) -> std::size_t;
    auto appendUnprepare(const std::string& block_name) -> std::size_t;
    [[nodiscard]] auto actions() const -> const std::vector<StatementAction>&;

  private:
    std::vector<StatementAction> actions_;
    std::unordered_map<std::string, StatementsBlock> prepared_blocks_;
};

} // namespace oink_judge::database
