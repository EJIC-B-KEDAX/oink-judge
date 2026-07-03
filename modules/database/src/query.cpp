#include "oink_judge/database/query.h"

#include "oink_judge/database/libpq_connection.h"

#include <oink_judge/logger/logger.h>

namespace oink_judge::database {

namespace {

constexpr const char* K_LOG_MODULE = "database";

using logger::logDebug;

} // namespace

namespace detail {

auto executePreparedPooled(ConnectionPool& pool, std::string stmt, std::vector<QueryParam> params, bool read_only) // NOLINT
    -> awaitable<QueryResult> {
    logDebug(K_LOG_MODULE, "Acquiring connection for prepared statement: " + stmt, 2);
    auto lease = co_await pool.acquire();
    co_return co_await lease.connection().executePrepared(std::move(stmt), params, read_only);
}

auto executeSQLPooled(ConnectionPool& pool, std::string sql, std::vector<QueryParam> params, bool read_only) // NOLINT
    -> awaitable<QueryResult> {
    logDebug(K_LOG_MODULE, "Acquiring connection for SQL query", 2);
    auto lease = co_await pool.acquire();
    co_return co_await lease.connection().executeSQL(std::move(sql), params, read_only);
}

} // namespace detail

auto execute(ConnectionPool& pool, std::string stmt, std::span<const QueryParam> params) -> awaitable<QueryResult> { // NOLINT
    co_return co_await detail::executePreparedPooled(pool, std::move(stmt),
                                                     std::vector<QueryParam>(params.begin(), params.end()), false);
}

auto executeReadOnly(ConnectionPool& pool, std::string stmt, std::span<const QueryParam> params) // NOLINT
    -> awaitable<QueryResult> {
    co_return co_await detail::executePreparedPooled(pool, std::move(stmt),
                                                     std::vector<QueryParam>(params.begin(), params.end()), true);
}

auto executeSQL(ConnectionPool& pool, std::string sql, std::span<const QueryParam> params) -> awaitable<QueryResult> { // NOLINT
    co_return co_await detail::executeSQLPooled(pool, std::move(sql), std::vector<QueryParam>(params.begin(), params.end()),
                                                false);
}

auto executeSQLReadOnly(ConnectionPool& pool, std::string sql, std::span<const QueryParam> params) // NOLINT
    -> awaitable<QueryResult> {
    co_return co_await detail::executeSQLPooled(pool, std::move(sql), std::vector<QueryParam>(params.begin(), params.end()),
                                                true);
}

auto quote(ConnectionPool& pool, std::string value) -> awaitable<std::string> { // NOLINT
    logDebug(K_LOG_MODULE, "Acquiring connection to quote literal value", 2);
    auto lease = co_await pool.acquire();
    co_return co_await lease.connection().quoteLiteral(std::move(value));
}

auto execute(ConnectionPool& pool, std::string stmt, const std::vector<QueryParam>& params) -> awaitable<QueryResult> { // NOLINT
    co_return co_await execute(pool, std::move(stmt), std::span<const QueryParam>(params));
}

auto executeReadOnly(ConnectionPool& pool, std::string stmt, const std::vector<QueryParam>& params) // NOLINT
    -> awaitable<QueryResult> {
    co_return co_await executeReadOnly(pool, std::move(stmt), std::span<const QueryParam>(params));
}

auto executeSQL(ConnectionPool& pool, std::string sql) -> awaitable<QueryResult> { // NOLINT
    co_return co_await executeSQL(pool, std::move(sql), std::span<const QueryParam>{});
}

auto execute(LibpqConnection& connection, std::string stmt, std::span<const QueryParam> params) -> awaitable<QueryResult> { // NOLINT
    co_return co_await connection.executePrepared(std::move(stmt), params, false);
}

auto executeReadOnly(LibpqConnection& connection, std::string stmt, std::span<const QueryParam> params) // NOLINT
    -> awaitable<QueryResult> {
    co_return co_await connection.executePrepared(std::move(stmt), params, true);
}

auto executeSQL(LibpqConnection& connection, std::string sql, std::span<const QueryParam> params) -> awaitable<QueryResult> { // NOLINT
    co_return co_await connection.executeSQL(std::move(sql), params, false);
}

auto executeSQLReadOnly(LibpqConnection& connection, std::string sql, std::span<const QueryParam> params) // NOLINT
    -> awaitable<QueryResult> {
    co_return co_await connection.executeSQL(std::move(sql), params, true);
}

auto execute(LibpqConnection& connection, std::string stmt, const std::vector<QueryParam>& params) -> awaitable<QueryResult> { // NOLINT
    co_return co_await execute(connection, std::move(stmt), std::span<const QueryParam>(params));
}

auto executeReadOnly(LibpqConnection& connection, std::string stmt, const std::vector<QueryParam>& params) // NOLINT
    -> awaitable<QueryResult> {
    co_return co_await executeReadOnly(connection, std::move(stmt), std::span<const QueryParam>(params));
}

auto executeSQL(LibpqConnection& connection, std::string sql) -> awaitable<QueryResult> { // NOLINT
    co_return co_await executeSQL(connection, std::move(sql), std::span<const QueryParam>{});
}

} // namespace oink_judge::database
