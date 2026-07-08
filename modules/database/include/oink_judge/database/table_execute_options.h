#pragma once
#include "oink_judge/database/database_executor_interface.h"
#include "oink_judge/database/execute_options.h"

namespace oink_judge::database {

struct TableExecuteOptions {
    ExecuteOptions execute_options;
    std::shared_ptr<DatabaseExecutorInterface> executor = nullptr;

    auto fillWithDefaults() -> void;
    auto fillWithDefaults(const TableExecuteOptions& default_options) -> void;
};

auto getDefaultExecutorOption() -> std::shared_ptr<DatabaseExecutorInterface>;

} // namespace oink_judge::database