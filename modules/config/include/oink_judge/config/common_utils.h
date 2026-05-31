#pragma once
#include <chrono>
#include <filesystem>
#include <optional>
#include <source_location>
#include <string>

namespace oink_judge::config {

namespace fs = std::filesystem;

template <typename T>
auto requireHasValue(const std::optional<T>& opt, const std::string& message = "optional expected with value",
                     const std::string& module = "optional_unpacking", int level = 1,
                     std::source_location location = std::source_location::current()) -> const T&;

auto getDirectoryPath(const std::string& key) -> std::optional<fs::path>;

auto getTiming(const std::string& timing_name) -> std::optional<std::chrono::duration<double>>;

} // namespace oink_judge::config

#include "oink_judge/config/common_utils.inl"
