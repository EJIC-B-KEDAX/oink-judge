#pragma once
#include "oink_judge/database/query_param.h"
#include "oink_judge/database/query_result.h"
#include "oink_judge/database/statement_action_log.h"

#include <boost/asio/any_io_executor.hpp>
#include <boost/asio/awaitable.hpp>
#include <boost/asio/posix/stream_descriptor.hpp>
#include <libpq-fe.h>

#include <cstddef>
#include <optional>
#include <span>
#include <string>

namespace oink_judge::database {

using boost::asio::awaitable;

class LibpqConnection {
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

    auto executePrepared(std::string stmt_name, std::span<const QueryParam> params, bool read_only) -> awaitable<QueryResult>;
    auto executeSQL(std::string sql, std::span<const QueryParam> params, bool read_only) -> awaitable<QueryResult>;
    auto quoteLiteral(std::string value) -> awaitable<std::string>;

    [[nodiscard]] auto isHealthy() const -> bool;

  private:
    PGconn* conn_{nullptr};
    std::string conninfo_;
    std::size_t statement_cursor_{0};
    int bound_libpq_fd_{-1};
    std::optional<boost::asio::posix::stream_descriptor> socket_;

    auto clearSocket() -> void;
    auto rebindSocketIfNeeded(const boost::asio::any_io_executor& executor) -> void;
    auto waitForPolling(PostgresPollingStatusType status) -> awaitable<void>;
    auto flushAsync() -> awaitable<void>;
    auto consumeResults() -> awaitable<QueryResult>;
    auto prepareAsync(std::string name, std::string sql) -> awaitable<void>;
    auto unprepareAsync(std::string name) -> awaitable<void>;
};

} // namespace oink_judge::database
