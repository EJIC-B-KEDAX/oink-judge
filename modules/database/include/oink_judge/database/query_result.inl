#pragma once
#include "oink_judge/database/query_result.h"

#include <cstdint>
#include <stdexcept>

namespace oink_judge::database {

template <> inline auto QueryField::as<std::string>() const -> std::string {
    if (isNull()) {
        throw std::runtime_error("null value cannot be converted to string");
    }
    return PQgetvalue(result_, row_, column_);
}

template <> inline auto QueryField::as<int>() const -> int { return std::stoi(as<std::string>()); }

template <> inline auto QueryField::as<std::int64_t>() const -> std::int64_t { return std::stoll(as<std::string>()); }

template <> inline auto QueryField::as<double>() const -> double { return std::stod(as<std::string>()); }

template <> inline auto QueryField::as<bool>() const -> bool {
    const auto value = as<std::string>();
    return value == "t" || value == "true" || value == "1";
}

} // namespace oink_judge::database
