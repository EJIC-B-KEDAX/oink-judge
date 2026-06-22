#pragma once
#include <optional>
#include <string>

namespace oink_judge::dispatcher {

auto getDispatcherStubType() -> std::optional<std::string>;

} // namespace oink_judge::dispatcher
