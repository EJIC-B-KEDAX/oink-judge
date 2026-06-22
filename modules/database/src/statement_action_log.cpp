#include "oink_judge/database/statement_action_log.h"

#include <stdexcept>

namespace oink_judge::database {

auto StatementActionLog::appendPrepare(std::string name, std::string sql) -> std::size_t {
    if (prepared_names_.contains(name)) {
        throw std::runtime_error("statement already prepared: " + name);
    }
    prepared_names_.insert(name);
    actions_.push_back(StatementAction{.type = StatementActionType::PREPARE, .name = std::move(name), .sql = std::move(sql)});
    return actions_.size();
}

auto StatementActionLog::appendUnprepare(std::string name) -> std::size_t {
    if (!prepared_names_.contains(name)) {
        throw std::runtime_error("statement not prepared: " + name);
    }
    prepared_names_.erase(name);
    actions_.push_back(StatementAction{.type = StatementActionType::UNPREPARE, .name = std::move(name), .sql = {}});
    return actions_.size();
}

auto StatementActionLog::actions() const -> const std::vector<StatementAction>& { return actions_; }

} // namespace oink_judge::database
