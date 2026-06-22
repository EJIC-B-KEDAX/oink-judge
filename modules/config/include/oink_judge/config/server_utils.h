#pragma once
#include <optional>

namespace oink_judge::config {

auto getMyPort() -> std::optional<int>;

} // namespace oink_judge::config
