#include "oink_judge/database/table_execute_options.h"

namespace oink_judge::database {

auto TableExecuteOptions::fillWithDefaults() -> void {
    execute_options.fillWithDefaults();
    if (executor == nullptr) {
        executor = getDefaultExecutorOption();
    }
}

auto TableExecuteOptions::fillWithDefaults(const TableExecuteOptions& default_options) -> void {
    execute_options.fillWithDefaults(default_options.execute_options);
    if (executor == nullptr) {
        executor = default_options.executor;
    }
}

auto getDefaultExecutorOption() -> std::shared_ptr<DatabaseExecutorInterface> { return getDefaultExecutor(); }

} // namespace oink_judge::database
