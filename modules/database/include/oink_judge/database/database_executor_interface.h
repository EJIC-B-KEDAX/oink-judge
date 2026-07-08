#pragma once
#include "oink_judge/database/execute_options.h"
#include "oink_judge/database/query_param.h"
#include "oink_judge/database/query_result.h"
#include "oink_judge/database/statements.h"

#include <oink_judge/factory/parameterized_type_factory.hpp>

#include <boost/asio/awaitable.hpp>

#include <string>
#include <type_traits>
#include <vector>

namespace oink_judge::database {

using boost::asio::awaitable;

class DatabaseExecutorInterface : public std::enable_shared_from_this<DatabaseExecutorInterface> {
  public:
    DatabaseExecutorInterface() = default;
    DatabaseExecutorInterface(const DatabaseExecutorInterface&) = delete;
    DatabaseExecutorInterface(DatabaseExecutorInterface&&) = delete;
    auto operator=(const DatabaseExecutorInterface&) -> DatabaseExecutorInterface& = delete;
    auto operator=(DatabaseExecutorInterface&&) -> DatabaseExecutorInterface& = delete;
    virtual ~DatabaseExecutorInterface() = default;

    virtual auto execute(ExecuteOptions options, std::string stmt, std::vector<QueryParam> params) -> awaitable<QueryResult> = 0;
    virtual auto executeSQL(ExecuteOptions options, std::string sql, std::vector<QueryParam> params)
        -> awaitable<QueryResult> = 0;
    virtual auto getConnection() -> awaitable<std::shared_ptr<DatabaseExecutorInterface>> = 0;
    virtual auto quote(std::string value) -> awaitable<std::string> = 0;
    virtual auto prepareStatements(StatementsBlock statements_block) -> awaitable<void> = 0;
    virtual auto unprepareStatements(std::string block_name) -> awaitable<void> = 0;
    [[nodiscard]] virtual auto isPrepared(const std::string& block_name) const -> bool = 0;

    template <typename... Args>
        requires((!std::same_as<std::decay_t<Args>, std::vector<QueryParam>> && ...))
    auto execute(std::string stmt, Args&&... args) -> awaitable<QueryResult>;

    template <typename... Args>
        requires((!std::same_as<std::decay_t<Args>, std::vector<QueryParam>> && ...))
    auto execute(ExecuteOptions options, std::string stmt, Args&&... args) -> awaitable<QueryResult>;

    template <typename... Args>
        requires((!std::same_as<std::decay_t<Args>, std::vector<QueryParam>> && ...))
    auto executeSQL(std::string sql, Args&&... args) -> awaitable<QueryResult>;

    template <typename... Args>
        requires((!std::same_as<std::decay_t<Args>, std::vector<QueryParam>> && ...))
    auto executeSQL(ExecuteOptions options, std::string sql, Args&&... args) -> awaitable<QueryResult>;

    auto prepareStatement(Statement statement) -> awaitable<void>;
    auto unprepareStatement(const std::string& statement_name) -> awaitable<void>;
};

auto createDatabaseExecutor(const std::string& name) -> std::shared_ptr<DatabaseExecutorInterface>;
auto getDefaultExecutor() -> std::shared_ptr<DatabaseExecutorInterface>;

using DatabaseExecutorFactory = factory::ParameterizedTypeFactory<std::shared_ptr<DatabaseExecutorInterface>>;

auto registerDatabaseExecutorTypes() -> void;

} // namespace oink_judge::database

#include "oink_judge/database/database_executor_interface.inl"
