#include "oink_judge/database/statements.h"

namespace oink_judge::database {

Statement::Statement(std::string name, std::string sql) : name_(std::move(name)), sql_(std::move(sql)) {}

auto Statement::name() const -> const std::string& { return name_; }
auto Statement::sql() const -> const std::string& { return sql_; }

StatementsBlock::StatementsBlock(std::string block_name) : block_name_(std::move(block_name)) {}

auto StatementsBlock::name() const -> const std::string& { return block_name_; }
auto StatementsBlock::addStatement(Statement statement) -> void {
    statements_.insert_or_assign(statement.name(), std::move(statement));
}
auto StatementsBlock::getStatement(const std::string& name) const -> const Statement& { return statements_.at(name); }
auto StatementsBlock::statements() const -> const std::unordered_map<std::string, Statement>& { return statements_; }

} // namespace oink_judge::database
