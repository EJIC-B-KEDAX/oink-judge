#pragma once
#include <filesystem>
#include <optional>
#include <string>

namespace oink_judge::content_service {

namespace fs = std::filesystem;

auto getContentDirectory(const std::string& content_type) -> std::optional<fs::path>;

} // namespace oink_judge::content_service
