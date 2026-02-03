#pragma once
#include <filesystem>
#include <string>

namespace oink_judge::utils::filesystem {

namespace fs = std::filesystem;

auto loadFile(const fs::path& path) -> std::string;

auto storeFile(const fs::path& path, const std::string& content) -> void;

auto createDirectoryIfNotExists(const fs::path& path) -> bool;

auto createFileIfNotExists(const fs::path& path) -> bool;

auto packDirectoryToZip(const fs::path& directory_path, const fs::path& zip_path) -> void;

auto unpackZipToDirectory(const fs::path& zip_path, const fs::path& directory_path) -> void;

auto removeFileOrDirectory(const fs::path& path) -> void;

auto clearDirectory(const fs::path& path) -> void;

} // namespace oink_judge::utils::filesystem
