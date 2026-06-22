#pragma once
#include "oink_judge/database/query_param.h"

namespace oink_judge::database {

template <typename T, typename... Rest> auto makeQueryParams(T&& first, Rest&&... rest) -> std::vector<QueryParam> {
    auto params = makeQueryParams(std::forward<Rest>(rest)...);
    params.insert(params.begin(), QueryParam(std::forward<T>(first)));
    return params;
}

template <typename... Args> auto makeQueryParamsVector(Args&&... args) -> std::vector<QueryParam> {
    return makeQueryParams(std::forward<Args>(args)...);
}

} // namespace oink_judge::database
