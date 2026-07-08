#include "oink_judge/database/statement_action_log.h"

#include <stdexcept>

namespace oink_judge::database {

auto StatementActionLog::appendPrepare(StatementsBlock block) -> std::size_t {
    const auto block_name = block.name();
    if (prepared_blocks_.contains(block_name)) {
        throw std::runtime_error("statements block already prepared: " + block_name);
    }
    prepared_blocks_.insert({block_name, std::move(block)});
    const auto& prepared_block = prepared_blocks_.at(block_name);
    actions_.push_back(StatementAction{.type = StatementActionType::PREPARE, .block = prepared_block});
    return actions_.size();
}

auto StatementActionLog::appendUnprepare(const std::string& block_name) -> std::size_t {
    if (!prepared_blocks_.contains(block_name)) {
        throw std::runtime_error("statements block not prepared: " + block_name);
    }
    auto block = prepared_blocks_.at(block_name);
    prepared_blocks_.erase(block_name);
    actions_.push_back(StatementAction{.type = StatementActionType::UNPREPARE, .block = std::move(block)});
    return actions_.size();
}

auto StatementActionLog::actions() const -> const std::vector<StatementAction>& { return actions_; }

} // namespace oink_judge::database
