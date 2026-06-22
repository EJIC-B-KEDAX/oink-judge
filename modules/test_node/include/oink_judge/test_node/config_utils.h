#pragma once
#include <filesystem>
#include <optional>
#include <string>

namespace oink_judge::test_node {

namespace fs = std::filesystem;

auto getMyTestNodeId() -> std::optional<std::string>;

auto getMyTestNodeType() -> std::optional<std::string>;

auto getTestingLogFilePath(const std::string& key) -> std::optional<fs::path>;

auto getQueueManagerServiceStubType() -> std::optional<std::string>;

} // namespace oink_judge::test_node
