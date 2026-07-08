#include "oink_judge/database/query.h"

#include <oink_judge/logger/logger.h>

namespace oink_judge::database {

namespace {

constexpr const char* K_LOG_MODULE = "database";

using logger::logDebug;

} // namespace

auto execute(ExecuteOptions options, std::string stmt, std::vector<QueryParam> params) -> awaitable<QueryResult> {
    auto executor = getDefaultExecutor();
    return executor->execute(options, std::move(stmt), std::move(params));
}

auto executeSQL(ExecuteOptions options, std::string sql, std::vector<QueryParam> params) -> awaitable<QueryResult> {
    auto executor = getDefaultExecutor();
    return executor->executeSQL(options, std::move(sql), std::move(params));
}

auto quote(std::string value) -> awaitable<std::string> {
    auto executor = getDefaultExecutor();
    return executor->quote(std::move(value));
}

} // namespace oink_judge::database
