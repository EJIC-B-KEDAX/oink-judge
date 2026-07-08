#include "oink_judge/database/execute_options.h"

#include "oink_judge/database/config_utils.h"

#include <oink_judge/config/common_utils.h>

namespace oink_judge::database {

auto ExecuteOptions::fillWithDefaults() -> void { fillWithDefaults(getDefaultExecuteOptions()); }

auto ExecuteOptions::fillWithDefaults(const ExecuteOptions& default_options) -> void {
    if (!read_only.has_value()) {
        read_only = default_options.read_only;
    }
    if (!timeout_sec.has_value()) {
        timeout_sec = default_options.timeout_sec;
    }
    if (!retries.has_value()) {
        retries = default_options.retries;
    }
}

auto getDefaultExecuteOptions() -> ExecuteOptions {
    const auto config = config::requireHasValue(getDatabaseConfig());

    ExecuteOptions result = {
        .read_only = config.query_read_only_default,
        .timeout_sec = config.query_timeout_sec,
        .retries = config.query_retries,
    };

    result.fillWithDefaults({.read_only = false, .timeout_sec = 0, .retries = 0});

    return result;
}

} // namespace oink_judge::database
