#pragma once
#include <optional>
#include <string>

namespace oink_judge::test_node {

auto getMyTestNodeId() -> std::optional<int>;

auto getTestingLogFilePath(const std::string& key) -> std::optional<std::string>;

} // namespace oink_judge::test_node
