#include "oink_judge/database/query.h"

#include "oink_judge/database/libpq_connection.h"

#include <oink_judge/logger/logger.h>

namespace oink_judge::database {

namespace {

constexpr const char* K_LOG_MODULE = "database";

using logger::logDebug;

} // namespace

namespace detail {

auto executePrepared(ConnectionPool& pool, std::string stmt, std::vector<QueryParam> params, bool read_only, // NOLINT
                     LibpqConnection* connection) -> awaitable<QueryResult> {
    if (connection != nullptr) {
        co_return co_await connection->executePrepared(std::move(stmt), params, read_only);
    }
    logDebug(K_LOG_MODULE, "Acquiring connection for prepared statement: " + stmt, 2);
    auto lease = co_await pool.acquire();
    co_return co_await lease.connection().executePrepared(std::move(stmt), params, read_only);
}

auto executeSQL(ConnectionPool& pool, std::string sql, std::vector<QueryParam> params, bool read_only, // NOLINT
                LibpqConnection* connection) -> awaitable<QueryResult> {
    if (connection != nullptr) {
        co_return co_await connection->executeSQL(std::move(sql), params, read_only);
    }
    logDebug(K_LOG_MODULE, "Acquiring connection for SQL query", 2);
    auto lease = co_await pool.acquire();
    co_return co_await lease.connection().executeSQL(std::move(sql), params, read_only);
}

} // namespace detail

auto execute(ConnectionPool& pool, std::string stmt, std::span<const QueryParam> params, LibpqConnection* connection)
    -> awaitable<QueryResult> { // NOLINT
    co_return co_await detail::executePrepared(pool, std::move(stmt), std::vector<QueryParam>(params.begin(), params.end()),
                                               false, connection);
}

auto executeReadOnly(ConnectionPool& pool, std::string stmt, std::span<const QueryParam> params, LibpqConnection* connection)
    -> awaitable<QueryResult> { // NOLINT
    co_return co_await detail::executePrepared(pool, std::move(stmt), std::vector<QueryParam>(params.begin(), params.end()),
                                               true, connection);
}

auto executeSQL(ConnectionPool& pool, std::string sql, std::span<const QueryParam> params, LibpqConnection* connection)
    -> awaitable<QueryResult> { // NOLINT
    co_return co_await detail::executeSQL(pool, std::move(sql), std::vector<QueryParam>(params.begin(), params.end()), false,
                                          connection);
}

auto executeSQLReadOnly(ConnectionPool& pool, std::string sql, std::span<const QueryParam> params, LibpqConnection* connection)
    -> awaitable<QueryResult> { // NOLINT
    co_return co_await detail::executeSQL(pool, std::move(sql), std::vector<QueryParam>(params.begin(), params.end()), true,
                                        connection);
}

auto quote(ConnectionPool& pool, std::string value) -> awaitable<std::string> { // NOLINT
    logDebug(K_LOG_MODULE, "Acquiring connection to quote literal value", 2);
    auto lease = co_await pool.acquire();
    co_return co_await lease.connection().quoteLiteral(std::move(value));
}

auto execute(ConnectionPool& pool, std::string stmt, const std::vector<QueryParam>& params, LibpqConnection* connection)
    -> awaitable<QueryResult> { // NOLINT
    co_return co_await execute(pool, std::move(stmt), std::span<const QueryParam>(params), connection);
}

auto executeReadOnly(ConnectionPool& pool, std::string stmt, const std::vector<QueryParam>& params, LibpqConnection* connection)
    -> awaitable<QueryResult> { // NOLINT
    co_return co_await executeReadOnly(pool, std::move(stmt), std::span<const QueryParam>(params), connection);
}

auto executeSQL(ConnectionPool& pool, std::string sql, LibpqConnection* connection) -> awaitable<QueryResult> { // NOLINT
    co_return co_await executeSQL(pool, std::move(sql), std::span<const QueryParam>{}, connection);
}

} // namespace oink_judge::database
