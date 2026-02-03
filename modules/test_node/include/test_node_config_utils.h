#pragma once
#include <optional>
#include <string>

namespace oink_judge::config {

auto getMyTestNodeId() -> std::optional<int>;

auto getTestingLogFilePath(const std::string& key) -> std::optional<std::string>;

} // namespace oink_judge::config
