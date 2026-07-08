#pragma once
#include <optional>

namespace oink_judge::database {

struct ExecuteOptions {
    std::optional<bool> read_only = std::nullopt;
    std::optional<int> timeout_sec = std::nullopt;
    std::optional<int> retries = std::nullopt;

    auto fillWithDefaults() -> void;
    auto fillWithDefaults(const ExecuteOptions& default_options) -> void;
};

auto getDefaultExecuteOptions() -> ExecuteOptions;

} // namespace oink_judge::database
