#pragma once
#include "oink_judge/database/connection_pool.h"
#include "oink_judge/database/query_param.h"
#include "oink_judge/database/query_result.h"

#include <boost/asio/awaitable.hpp>

#include <concepts>
#include <span>
#include <string>
#include <type_traits>
#include <vector>

namespace oink_judge::database {

using boost::asio::awaitable;

class LibpqConnection;

// A single LibpqConnection supports one in-flight query at a time. Concurrent
// execute/executeSQL calls on the same connection interleave on the shared PGconn
// and corrupt the libpq protocol (mixed results, errors). Serialize queries on one
// connection with co_await, or use separate connections from the pool.

namespace detail {
auto executePreparedPooled(ConnectionPool& pool, std::string stmt, std::vector<QueryParam> params, bool read_only)
    -> awaitable<QueryResult>;
auto executeSQLPooled(ConnectionPool& pool, std::string sql, std::vector<QueryParam> params, bool read_only)
    -> awaitable<QueryResult>;
} // namespace detail

// Acquires a connection from the pool for each call.
auto execute(ConnectionPool& pool, std::string stmt, std::span<const QueryParam> params) -> awaitable<QueryResult>;
auto executeReadOnly(ConnectionPool& pool, std::string stmt, std::span<const QueryParam> params) -> awaitable<QueryResult>;
auto executeSQL(ConnectionPool& pool, std::string sql, std::span<const QueryParam> params) -> awaitable<QueryResult>;
auto executeSQLReadOnly(ConnectionPool& pool, std::string sql, std::span<const QueryParam> params) -> awaitable<QueryResult>;
auto quote(ConnectionPool& pool, std::string value) -> awaitable<std::string>;
auto quote(LibpqConnection& connection, std::string value) -> awaitable<std::string>;

auto execute(ConnectionPool& pool, std::string stmt, const std::vector<QueryParam>& params) -> awaitable<QueryResult>;
auto executeReadOnly(ConnectionPool& pool, std::string stmt, const std::vector<QueryParam>& params) -> awaitable<QueryResult>;
auto executeSQL(ConnectionPool& pool, std::string sql) -> awaitable<QueryResult>;

// Uses ConnectionPool::instance(); acquires a connection from the default pool for each call.
auto execute(std::string stmt, std::span<const QueryParam> params) -> awaitable<QueryResult>;
auto executeReadOnly(std::string stmt, std::span<const QueryParam> params) -> awaitable<QueryResult>;
auto executeSQL(std::string sql, std::span<const QueryParam> params) -> awaitable<QueryResult>;
auto executeSQLReadOnly(std::string sql, std::span<const QueryParam> params) -> awaitable<QueryResult>;
auto quote(std::string value) -> awaitable<std::string>;

auto execute(std::string stmt, const std::vector<QueryParam>& params) -> awaitable<QueryResult>;
auto executeReadOnly(std::string stmt, const std::vector<QueryParam>& params) -> awaitable<QueryResult>;
auto executeSQL(std::string sql) -> awaitable<QueryResult>;

// Runs on an already-acquired connection (caller keeps the lease alive).
auto execute(LibpqConnection& connection, std::string stmt, std::span<const QueryParam> params) -> awaitable<QueryResult>;
auto executeReadOnly(LibpqConnection& connection, std::string stmt, std::span<const QueryParam> params) -> awaitable<QueryResult>;
auto executeSQL(LibpqConnection& connection, std::string sql, std::span<const QueryParam> params) -> awaitable<QueryResult>;
auto executeSQLReadOnly(LibpqConnection& connection, std::string sql, std::span<const QueryParam> params)
    -> awaitable<QueryResult>;

auto execute(LibpqConnection& connection, std::string stmt, const std::vector<QueryParam>& params) -> awaitable<QueryResult>;
auto executeReadOnly(LibpqConnection& connection, std::string stmt, const std::vector<QueryParam>& params)
    -> awaitable<QueryResult>;
auto executeSQL(LibpqConnection& connection, std::string sql) -> awaitable<QueryResult>;

template <typename... Args>
    requires((!std::same_as<std::decay_t<Args>, std::vector<QueryParam>> && ...))
auto execute(ConnectionPool& pool, std::string stmt, Args&&... args) -> awaitable<QueryResult>;
template <typename... Args>
    requires((!std::same_as<std::decay_t<Args>, std::vector<QueryParam>> && ...))
auto executeReadOnly(ConnectionPool& pool, std::string stmt, Args&&... args) -> awaitable<QueryResult>;
template <typename... Args>
    requires((!std::same_as<std::decay_t<Args>, std::vector<QueryParam>> && ...))
auto executeSQL(ConnectionPool& pool, std::string sql, Args&&... args) -> awaitable<QueryResult>;
template <typename... Args>
    requires((!std::same_as<std::decay_t<Args>, std::vector<QueryParam>> && ...))
auto executeSQLReadOnly(ConnectionPool& pool, std::string sql, Args&&... args) -> awaitable<QueryResult>;

template <typename... Args>
    requires((!std::same_as<std::decay_t<Args>, std::vector<QueryParam>> && ...))
auto execute(std::string stmt, Args&&... args) -> awaitable<QueryResult>;
template <typename... Args>
    requires((!std::same_as<std::decay_t<Args>, std::vector<QueryParam>> && ...))
auto executeReadOnly(std::string stmt, Args&&... args) -> awaitable<QueryResult>;
template <typename... Args>
    requires((!std::same_as<std::decay_t<Args>, std::vector<QueryParam>> && ...))
auto executeSQL(std::string sql, Args&&... args) -> awaitable<QueryResult>;
template <typename... Args>
    requires((!std::same_as<std::decay_t<Args>, std::vector<QueryParam>> && ...))
auto executeSQLReadOnly(std::string sql, Args&&... args) -> awaitable<QueryResult>;

template <typename... Args>
    requires((!std::same_as<std::decay_t<Args>, std::vector<QueryParam>> && ...))
auto execute(LibpqConnection& connection, std::string stmt, Args&&... args) -> awaitable<QueryResult>;
template <typename... Args>
    requires((!std::same_as<std::decay_t<Args>, std::vector<QueryParam>> && ...))
auto executeReadOnly(LibpqConnection& connection, std::string stmt, Args&&... args) -> awaitable<QueryResult>;
template <typename... Args>
    requires((!std::same_as<std::decay_t<Args>, std::vector<QueryParam>> && ...))
auto executeSQL(LibpqConnection& connection, std::string sql, Args&&... args) -> awaitable<QueryResult>;
template <typename... Args>
    requires((!std::same_as<std::decay_t<Args>, std::vector<QueryParam>> && ...))
auto executeSQLReadOnly(LibpqConnection& connection, std::string sql, Args&&... args) -> awaitable<QueryResult>;

} // namespace oink_judge::database

#include "oink_judge/database/query.inl"
