#pragma once
#include "oink_judge/database/database_executor_interface.h"
#include "oink_judge/database/statement_action_log.h"

#include <boost/asio/any_io_executor.hpp>
#include <boost/asio/awaitable.hpp>
#include <boost/asio/posix/stream_descriptor.hpp>
#include <boost/asio/steady_timer.hpp>
#include <libpq-fe.h>

#include <cstddef>
#include <deque>
#include <memory>
#include <mutex>
#include <optional>
#include <span>
#include <string>
#include <unordered_set>
#include <vector>

namespace oink_judge::database {

using boost::asio::awaitable;

class LibpqConnection : public DatabaseExecutorInterface {
  public:
    LibpqConnection();
    ~LibpqConnection();

    LibpqConnection(const LibpqConnection&) = delete;
    auto operator=(const LibpqConnection&) -> LibpqConnection& = delete;
    LibpqConnection(LibpqConnection&&) = delete;
    auto operator=(LibpqConnection&&) -> LibpqConnection& = delete;

    auto connectAsync(std::string conninfo) -> awaitable<void>;
    auto reconnectAsync(std::string conninfo) -> awaitable<void>;

    auto syncStatements(std::span<const StatementAction> actions) -> awaitable<void>;

    auto execute(ExecuteOptions options, std::string stmt, std::vector<QueryParam> params) -> awaitable<QueryResult> override;
    auto executeSQL(ExecuteOptions options, std::string sql, std::vector<QueryParam> params) -> awaitable<QueryResult> override;
    auto getConnection() -> awaitable<std::shared_ptr<DatabaseExecutorInterface>> override;
    auto quote(std::string value) -> awaitable<std::string> override;
    auto prepareStatements(StatementsBlock statements_block) -> awaitable<void> override;
    auto unprepareStatements(std::string block_name) -> awaitable<void> override;
    [[nodiscard]] auto isPrepared(const std::string& block_name) const -> bool override;

    auto executePrepared(std::string stmt_name, std::span<const QueryParam> params, bool read_only) -> awaitable<QueryResult>;
    auto executeRawSQL(std::string sql, std::span<const QueryParam> params, bool read_only) -> awaitable<QueryResult>;
    auto quoteLiteral(std::string value) -> awaitable<std::string>;

    [[nodiscard]] auto isHealthy() const -> bool;

  private:
    class ExecutionQueueLease {
      public:
        explicit ExecutionQueueLease(LibpqConnection* owner = nullptr) noexcept;
        ~ExecutionQueueLease();

        ExecutionQueueLease(const ExecutionQueueLease&) = delete;
        auto operator=(const ExecutionQueueLease&) -> ExecutionQueueLease& = delete;
        ExecutionQueueLease(ExecutionQueueLease&& other) noexcept;
        auto operator=(ExecutionQueueLease&& other) noexcept -> ExecutionQueueLease&;

      private:
        LibpqConnection* owner_;
    };

    PGconn* conn_{nullptr};
    std::string conninfo_;
    std::size_t statement_cursor_{0};
    int bound_libpq_fd_{-1};
    std::optional<boost::asio::posix::stream_descriptor> socket_;
    std::unordered_set<std::string> prepared_blocks_;
    bool execution_in_progress_{false};
    std::deque<std::shared_ptr<boost::asio::steady_timer>> execution_waiters_;
    mutable std::mutex execution_queue_mutex_;

    auto clearSocket() -> void;
    auto acquireExecutionQueue() -> awaitable<ExecutionQueueLease>;
    auto releaseExecutionQueue() -> void;
    auto rebindSocketIfNeeded(const boost::asio::any_io_executor& executor) -> void;
    auto waitForPolling(PostgresPollingStatusType status) -> awaitable<void>;
    auto flushAsync() -> awaitable<void>;
    auto consumeResults() -> awaitable<QueryResult>;
    auto prepareAsync(std::string name, std::string sql) -> awaitable<void>;
    auto unprepareAsync(std::string name) -> awaitable<void>;
};

} // namespace oink_judge::database
