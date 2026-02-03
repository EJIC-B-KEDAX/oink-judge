#pragma once
#include <optional>
#include <string>

namespace oink_judge::config {

auto getMyPort() -> std::optional<int>;

auto getConnectionHandlerType() -> std::optional<std::string>;

} // namespace oink_judge::config
