#pragma once
#include "oink_judge/database/database_executor_interface.h"
#include "oink_judge/database/libpq_connection.h"

#include <cstddef>

namespace oink_judge::database {

class ConnectionPool;

class PooledConnection : public DatabaseExecutorInterface {
  public:
    PooledConnection(std::shared_ptr<ConnectionPool> pool, std::size_t slot_index);
    ~PooledConnection();

    PooledConnection(const PooledConnection&) = delete;
    auto operator=(const PooledConnection&) -> PooledConnection& = delete;
    PooledConnection(PooledConnection&& other) noexcept;
    auto operator=(PooledConnection&& other) noexcept -> PooledConnection&;

    [[nodiscard]] auto connection() const -> std::shared_ptr<LibpqConnection>;

    auto execute(ExecuteOptions options, std::string stmt, std::vector<QueryParam> params) -> awaitable<QueryResult> override;
    auto executeSQL(ExecuteOptions options, std::string sql, std::vector<QueryParam> params) -> awaitable<QueryResult> override;
    auto getConnection() -> awaitable<std::shared_ptr<DatabaseExecutorInterface>> override;
    auto quote(std::string value) -> awaitable<std::string> override;
    auto prepareStatements(StatementsBlock statements_block) -> awaitable<void> override;
    auto unprepareStatements(std::string block_name) -> awaitable<void> override;
    [[nodiscard]] auto isPrepared(const std::string& block_name) const -> bool override;

  private:
    std::shared_ptr<ConnectionPool> pool_;
    std::size_t slot_index_;
};

} // namespace oink_judge::database
