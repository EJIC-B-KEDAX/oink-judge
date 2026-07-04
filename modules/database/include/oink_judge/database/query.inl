#pragma once
#include "oink_judge/database/query.h"

#include "oink_judge/database/libpq_connection.h"

#include <concepts>
#include <type_traits>

namespace oink_judge::database {

template <typename... Args>
    requires((!std::same_as<std::decay_t<Args>, std::vector<QueryParam>> && ...))
auto execute(ConnectionPool& pool, std::string stmt, Args&&... args) -> awaitable<QueryResult> {
    return detail::executePreparedPooled(pool, std::move(stmt), makeQueryParams(std::forward<Args>(args)...), false);
}

template <typename... Args>
    requires((!std::same_as<std::decay_t<Args>, std::vector<QueryParam>> && ...))
auto executeReadOnly(ConnectionPool& pool, std::string stmt, Args&&... args) -> awaitable<QueryResult> {
    return detail::executePreparedPooled(pool, std::move(stmt), makeQueryParams(std::forward<Args>(args)...), true);
}

template <typename... Args>
    requires((!std::same_as<std::decay_t<Args>, std::vector<QueryParam>> && ...))
auto executeSQL(ConnectionPool& pool, std::string sql, Args&&... args) -> awaitable<QueryResult> {
    return detail::executeSQLPooled(pool, std::move(sql), makeQueryParams(std::forward<Args>(args)...), false);
}

template <typename... Args>
    requires((!std::same_as<std::decay_t<Args>, std::vector<QueryParam>> && ...))
auto executeSQLReadOnly(ConnectionPool& pool, std::string sql, Args&&... args) -> awaitable<QueryResult> {
    return detail::executeSQLPooled(pool, std::move(sql), makeQueryParams(std::forward<Args>(args)...), true);
}

template <typename... Args>
    requires((!std::same_as<std::decay_t<Args>, std::vector<QueryParam>> && ...))
auto execute(std::string stmt, Args&&... args) -> awaitable<QueryResult> {
    return detail::executePreparedPooled(ConnectionPool::instance(), std::move(stmt),
                                           makeQueryParams(std::forward<Args>(args)...), false);
}

template <typename... Args>
    requires((!std::same_as<std::decay_t<Args>, std::vector<QueryParam>> && ...))
auto executeReadOnly(std::string stmt, Args&&... args) -> awaitable<QueryResult> {
    return detail::executePreparedPooled(ConnectionPool::instance(), std::move(stmt),
                                           makeQueryParams(std::forward<Args>(args)...), true);
}

template <typename... Args>
    requires((!std::same_as<std::decay_t<Args>, std::vector<QueryParam>> && ...))
auto executeSQL(std::string sql, Args&&... args) -> awaitable<QueryResult> {
    return detail::executeSQLPooled(ConnectionPool::instance(), std::move(sql), makeQueryParams(std::forward<Args>(args)...),
                                    false);
}

template <typename... Args>
    requires((!std::same_as<std::decay_t<Args>, std::vector<QueryParam>> && ...))
auto executeSQLReadOnly(std::string sql, Args&&... args) -> awaitable<QueryResult> {
    return detail::executeSQLPooled(ConnectionPool::instance(), std::move(sql), makeQueryParams(std::forward<Args>(args)...),
                                    true);
}

template <typename... Args>
    requires((!std::same_as<std::decay_t<Args>, std::vector<QueryParam>> && ...))
auto execute(LibpqConnection& connection, std::string stmt, Args&&... args) -> awaitable<QueryResult> {
    return connection.executePrepared(std::move(stmt), makeQueryParams(std::forward<Args>(args)...), false);
}

template <typename... Args>
    requires((!std::same_as<std::decay_t<Args>, std::vector<QueryParam>> && ...))
auto executeReadOnly(LibpqConnection& connection, std::string stmt, Args&&... args) -> awaitable<QueryResult> {
    return connection.executePrepared(std::move(stmt), makeQueryParams(std::forward<Args>(args)...), true);
}

template <typename... Args>
    requires((!std::same_as<std::decay_t<Args>, std::vector<QueryParam>> && ...))
auto executeSQL(LibpqConnection& connection, std::string sql, Args&&... args) -> awaitable<QueryResult> {
    return connection.executeSQL(std::move(sql), makeQueryParams(std::forward<Args>(args)...), false);
}

template <typename... Args>
    requires((!std::same_as<std::decay_t<Args>, std::vector<QueryParam>> && ...))
auto executeSQLReadOnly(LibpqConnection& connection, std::string sql, Args&&... args) -> awaitable<QueryResult> {
    return connection.executeSQL(std::move(sql), makeQueryParams(std::forward<Args>(args)...), true);
}

} // namespace oink_judge::database
