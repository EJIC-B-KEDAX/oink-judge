#include "oink_judge/database/database_executor_interface.h"

#include "oink_judge/database/config_utils.h"
#include "oink_judge/database/connection_pool.h"

namespace oink_judge::database {

auto DatabaseExecutorInterface::prepareStatement(Statement statement) -> awaitable<void> {
    StatementsBlock statements_block(statement.name());
    statements_block.addStatement(std::move(statement));
    return prepareStatements(std::move(statements_block));
}

auto DatabaseExecutorInterface::unprepareStatement(const std::string& statement_name) -> awaitable<void> {
    return unprepareStatements(statement_name);
}

namespace {

auto ensureDatabaseExecutorTypesRegistered() -> void {
    static const bool registered = []() -> bool {
        registerDatabaseExecutorTypes();
        return true;
    }();
    (void)registered;
}

} // namespace

auto registerDatabaseExecutorTypes() -> void {
    DatabaseExecutorFactory::instance().registerType(ConnectionPool::REGISTERED_NAME,
                                                     [](const std::string& params) -> std::shared_ptr<DatabaseExecutorInterface> {
                                                         return std::make_shared<ConnectionPool>();
                                                     });
}

auto createDatabaseExecutor(const std::string& name) -> std::shared_ptr<DatabaseExecutorInterface> {
    ensureDatabaseExecutorTypesRegistered();
    return DatabaseExecutorFactory::instance().create(name);
}

auto getDefaultExecutor() -> std::shared_ptr<DatabaseExecutorInterface> {
    static auto executor = createDatabaseExecutor(getDefaultExecutorName());
    return executor;
}

} // namespace oink_judge::database
