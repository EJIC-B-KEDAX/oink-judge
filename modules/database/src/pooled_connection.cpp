#include "oink_judge/database/pooled_connection.h"

#include "oink_judge/database/connection_pool.h"

namespace oink_judge::database {

PooledConnection::PooledConnection(std::shared_ptr<ConnectionPool> pool, std::size_t slot_index)
    : pool_(std::move(pool)), slot_index_(slot_index) {}

PooledConnection::~PooledConnection() {
    if (pool_ != nullptr) {
        pool_->releaseSlot(slot_index_);
    }
}

PooledConnection::PooledConnection(PooledConnection&& other) noexcept
    : pool_(std::move(other.pool_)), slot_index_(other.slot_index_) {
    other.pool_ = nullptr;
}

auto PooledConnection::operator=(PooledConnection&& other) noexcept -> PooledConnection& {
    if (this != &other) {
        if (pool_ != nullptr) {
            pool_->releaseSlot(slot_index_);
        }
        pool_ = std::move(other.pool_);
        slot_index_ = other.slot_index_;
        other.pool_ = nullptr;
    }
    return *this;
}

auto PooledConnection::connection() const -> std::shared_ptr<LibpqConnection> { return pool_->connectionAt(slot_index_); }

auto PooledConnection::execute(ExecuteOptions options, std::string stmt, std::vector<QueryParam> params)
    -> awaitable<QueryResult> {
    co_return co_await connection()->execute(options, std::move(stmt), std::move(params));
}

auto PooledConnection::executeSQL(ExecuteOptions options, std::string sql, std::vector<QueryParam> params)
    -> awaitable<QueryResult> {
    co_return co_await connection()->executeSQL(options, std::move(sql), std::move(params));
}

auto PooledConnection::getConnection() -> awaitable<std::shared_ptr<DatabaseExecutorInterface>> {
    co_return std::static_pointer_cast<DatabaseExecutorInterface>(shared_from_this());
}

auto PooledConnection::quote(std::string value) -> awaitable<std::string> {
    co_return co_await connection()->quote(std::move(value));
}

auto PooledConnection::prepareStatements(StatementsBlock statements_block) -> awaitable<void> {
    co_await pool_->prepareStatements(std::move(statements_block));
    const auto actions = pool_->copyStatementActions();
    co_await connection()->syncStatements(actions);
    co_return;
}

auto PooledConnection::unprepareStatements(std::string block_name) -> awaitable<void> {
    co_await pool_->unprepareStatements(std::move(block_name));
    const auto actions = pool_->copyStatementActions();
    co_await connection()->syncStatements(actions);
    co_return;
}

auto PooledConnection::isPrepared(const std::string& block_name) const -> bool { return connection()->isPrepared(block_name); }

} // namespace oink_judge::database
