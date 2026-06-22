#pragma once
#include "oink_judge/database/query.h"

#include <concepts>
#include <type_traits>

namespace oink_judge::database {

template <typename... Args>
    requires((!std::same_as<std::decay_t<Args>, std::vector<QueryParam>> && ...))
auto execute(ConnectionPool& pool, std::string stmt, Args&&... args) -> awaitable<QueryResult> {
    return detail::executePrepared(pool, std::move(stmt), makeQueryParams(std::forward<Args>(args)...), false);
}

template <typename... Args>
    requires((!std::same_as<std::decay_t<Args>, std::vector<QueryParam>> && ...))
auto executeReadOnly(ConnectionPool& pool, std::string stmt, Args&&... args) -> awaitable<QueryResult> {
    return detail::executePrepared(pool, std::move(stmt), makeQueryParams(std::forward<Args>(args)...), true);
}

template <typename... Args>
    requires((!std::same_as<std::decay_t<Args>, std::vector<QueryParam>> && ...))
auto executeSQL(ConnectionPool& pool, std::string sql, Args&&... args) -> awaitable<QueryResult> {
    return detail::executeSQL(pool, std::move(sql), makeQueryParams(std::forward<Args>(args)...), false);
}

template <typename... Args>
    requires((!std::same_as<std::decay_t<Args>, std::vector<QueryParam>> && ...))
auto executeSQLReadOnly(ConnectionPool& pool, std::string sql, Args&&... args) -> awaitable<QueryResult> {
    return detail::executeSQL(pool, std::move(sql), makeQueryParams(std::forward<Args>(args)...), true);
}

} // namespace oink_judge::database
