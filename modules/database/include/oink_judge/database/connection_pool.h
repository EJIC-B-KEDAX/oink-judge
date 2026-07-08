#pragma once
#include "oink_judge/database/config_utils.h"
#include "oink_judge/database/database_executor_interface.h"
#include "oink_judge/database/libpq_connection.h"
#include "oink_judge/database/pooled_connection.h"
#include "oink_judge/database/statement_action_log.h"

#include <boost/asio/any_io_executor.hpp>
#include <boost/asio/awaitable.hpp>

#include <cstddef>
#include <memory>
#include <mutex>
#include <optional>
#include <string>
#include <unordered_set>
#include <vector>

namespace oink_judge::database {

using boost::asio::awaitable;

class ConnectionPool : public DatabaseExecutorInterface {
  public:
    explicit ConnectionPool(std::optional<DatabaseConfig> config = std::nullopt);

    auto initialize() -> awaitable<void>;

    auto acquire() -> awaitable<std::shared_ptr<PooledConnection>>;

    auto execute(ExecuteOptions options, std::string stmt, std::vector<QueryParam> params) -> awaitable<QueryResult> override;
    auto executeSQL(ExecuteOptions options, std::string sql, std::vector<QueryParam> params) -> awaitable<QueryResult> override;
    auto getConnection() -> awaitable<std::shared_ptr<DatabaseExecutorInterface>> override;
    auto quote(std::string value) -> awaitable<std::string> override;
    auto prepareStatements(StatementsBlock statements_block) -> awaitable<void> override;
    auto unprepareStatements(std::string block_name) -> awaitable<void> override;
    [[nodiscard]] auto isPrepared(const std::string& block_name) const -> bool override;

    static constexpr const char* REGISTERED_NAME = "connection_pool";

  private:
    friend class PooledConnection;

    struct Slot {
        std::shared_ptr<LibpqConnection> connection;
        bool in_use = false;
    };

    auto requireInitialized() -> awaitable<void>;
    auto connectionAt(std::size_t slot_index) -> std::shared_ptr<LibpqConnection>;
    auto releaseSlot(std::size_t slot_index) -> void;
    auto createSlot() -> awaitable<std::size_t>;
    auto tryAcquireExistingSlot() -> std::optional<std::size_t>;
    auto waitForAvailableSlot() -> awaitable<std::size_t>;
    auto copyStatementActions() const -> std::vector<StatementAction>;

    DatabaseConfig config_;
    std::string conninfo_;
    bool initialized_;
    StatementActionLog statement_log_;
    std::vector<Slot> slots_;
    mutable std::mutex pool_mutex_;
    mutable std::mutex statement_log_mutex_;
    boost::asio::any_io_executor executor_;
    std::unordered_set<std::string> prepared_blocks_;
};

} // namespace oink_judge::database
