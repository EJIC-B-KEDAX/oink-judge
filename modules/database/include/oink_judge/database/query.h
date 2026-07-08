#pragma once
#include "oink_judge/database/database_executor_interface.h"
#include "oink_judge/database/execute_options.h"
#include "oink_judge/database/query_param.h"
#include "oink_judge/database/query_result.h"

#include <boost/asio/awaitable.hpp>

#include <concepts>
#include <string>
#include <type_traits>
#include <vector>

namespace oink_judge::database {

using boost::asio::awaitable;

class LibpqConnection;

// Queries issued through a single LibpqConnection are queued and executed one at
// a time. Separate pooled connections can still run concurrently.

auto execute(ExecuteOptions options, std::string stmt, std::vector<QueryParam> params) -> awaitable<QueryResult>;
auto executeSQL(ExecuteOptions options, std::string sql, std::vector<QueryParam> params) -> awaitable<QueryResult>;
auto quote(std::string value) -> awaitable<std::string>;

template <typename... Args>
    requires((!std::same_as<std::decay_t<Args>, std::vector<QueryParam>> && ...))
auto execute(std::string stmt, Args&&... args) -> awaitable<QueryResult>;
template <typename... Args>
    requires((!std::same_as<std::decay_t<Args>, std::vector<QueryParam>> && ...))
auto executeSQL(std::string sql, Args&&... args) -> awaitable<QueryResult>;

template <typename... Args>
    requires((!std::same_as<std::decay_t<Args>, std::vector<QueryParam>> && ...))
auto execute(ExecuteOptions options, std::string stmt, Args&&... args) -> awaitable<QueryResult>;
template <typename... Args>
    requires((!std::same_as<std::decay_t<Args>, std::vector<QueryParam>> && ...))
auto executeSQL(ExecuteOptions options, std::string sql, Args&&... args) -> awaitable<QueryResult>;

} // namespace oink_judge::database

#include "oink_judge/database/query.inl"
