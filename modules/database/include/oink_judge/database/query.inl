#pragma once
#include "oink_judge/database/query.h"

#include <concepts>
#include <type_traits>

namespace oink_judge::database {

template <typename... Args>
    requires((!std::same_as<std::decay_t<Args>, std::vector<QueryParam>> && ...))
auto execute(std::string stmt, Args&&... args) -> awaitable<QueryResult> {
    ExecuteOptions options = getDefaultExecuteOptions();
    return execute(options, std::move(stmt), std::forward<Args>(args)...);
}

template <typename... Args>
    requires((!std::same_as<std::decay_t<Args>, std::vector<QueryParam>> && ...))
auto executeSQL(std::string sql, Args&&... args) -> awaitable<QueryResult> {
    ExecuteOptions options = getDefaultExecuteOptions();
    return executeSQL(options, std::move(sql), std::forward<Args>(args)...);
}

template <typename... Args>
    requires((!std::same_as<std::decay_t<Args>, std::vector<QueryParam>> && ...))
auto execute(ExecuteOptions options, std::string stmt, Args&&... args) -> awaitable<QueryResult> {
    auto executor = getDefaultExecutor();
    return executor->execute(options, std::move(stmt), std::forward<Args>(args)...);
}

template <typename... Args>
    requires((!std::same_as<std::decay_t<Args>, std::vector<QueryParam>> && ...))
auto executeSQL(ExecuteOptions options, std::string sql, Args&&... args) -> awaitable<QueryResult> {
    auto executor = getDefaultExecutor();
    return executor->executeSQL(options, std::move(sql), std::forward<Args>(args)...);
}

} // namespace oink_judge::database
