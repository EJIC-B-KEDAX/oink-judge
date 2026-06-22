#pragma once
#include <cstdint>
#include <string>
#include <variant>
#include <vector>

namespace oink_judge::database {

using QueryParam = std::variant<std::monostate, bool, std::int32_t, std::int64_t, double, std::string>;

auto makeQueryParams() -> std::vector<QueryParam>;

template <typename T, typename... Rest> auto makeQueryParams(T&& first, Rest&&... rest) -> std::vector<QueryParam>;

template <typename... Args> auto makeQueryParamsVector(Args&&... args) -> std::vector<QueryParam>;

} // namespace oink_judge::database

#include "oink_judge/database/query_param.inl"
