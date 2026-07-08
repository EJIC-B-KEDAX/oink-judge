#pragma once
#include "oink_judge/database/database_executor_interface.h"

namespace oink_judge::database {

template <typename... Args>
    requires((!std::same_as<std::decay_t<Args>, std::vector<QueryParam>> && ...))
auto DatabaseExecutorInterface::execute(std::string stmt, Args&&... args) -> awaitable<QueryResult> {
    return execute({}, std::move(stmt), makeQueryParams(std::forward<Args>(args)...));
}

template <typename... Args>
    requires((!std::same_as<std::decay_t<Args>, std::vector<QueryParam>> && ...))
auto DatabaseExecutorInterface::execute(ExecuteOptions options, std::string stmt, Args&&... args) -> awaitable<QueryResult> {
    return execute(options, std::move(stmt), makeQueryParams(std::forward<Args>(args)...));
}

template <typename... Args>
    requires((!std::same_as<std::decay_t<Args>, std::vector<QueryParam>> && ...))
auto DatabaseExecutorInterface::executeSQL(std::string sql, Args&&... args) -> awaitable<QueryResult> {
    return executeSQL({}, std::move(sql), makeQueryParams(std::forward<Args>(args)...));
}

template <typename... Args>
    requires((!std::same_as<std::decay_t<Args>, std::vector<QueryParam>> && ...))
auto DatabaseExecutorInterface::executeSQL(ExecuteOptions options, std::string sql, Args&&... args) -> awaitable<QueryResult> {
    return executeSQL(options, std::move(sql), makeQueryParams(std::forward<Args>(args)...));
}

} // namespace oink_judge::database
