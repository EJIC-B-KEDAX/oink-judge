#pragma once
#include <string>

namespace oink_judge::utils::crypto {

auto sha256(const std::string& input) -> std::string;

auto toBase64(const std::string& input) -> std::string;

auto fromBase64(const std::string& input) -> std::string;

} // namespace oink_judge::utils::crypto
