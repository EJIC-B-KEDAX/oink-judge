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

namespace detail {
auto executePrepared(ConnectionPool& pool, std::string stmt, std::vector<QueryParam> params, bool read_only,
                     LibpqConnection* connection = nullptr) -> awaitable<QueryResult>;
auto executeSQL(ConnectionPool& pool, std::string sql, std::vector<QueryParam> params, bool read_only,
                LibpqConnection* connection = nullptr) -> awaitable<QueryResult>;
} // namespace detail

auto execute(ConnectionPool& pool, std::string stmt, std::span<const QueryParam> params) -> awaitable<QueryResult>;
auto executeReadOnly(ConnectionPool& pool, std::string stmt, std::span<const QueryParam> params) -> awaitable<QueryResult>;
auto executeSQL(ConnectionPool& pool, std::string sql, std::span<const QueryParam> params) -> awaitable<QueryResult>;
auto executeSQLReadOnly(ConnectionPool& pool, std::string sql, std::span<const QueryParam> params) -> awaitable<QueryResult>;
auto quote(ConnectionPool& pool, std::string value) -> awaitable<std::string>;

auto execute(ConnectionPool& pool, std::string stmt, const std::vector<QueryParam>& params) -> awaitable<QueryResult>;
auto executeReadOnly(ConnectionPool& pool, std::string stmt, const std::vector<QueryParam>& params) -> awaitable<QueryResult>;
auto executeSQL(ConnectionPool& pool, std::string sql) -> awaitable<QueryResult>;

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

} // namespace oink_judge::database

#include "oink_judge/database/query.inl"
