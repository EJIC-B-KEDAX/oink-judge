#pragma once
#include "Protocol.hpp"

namespace oink_judge::socket {

template<typename... Args>
void Protocol::call_callback(const callback_t &callback, std::error_code ec, Args&&... args) {
    if constexpr (sizeof...(Args) == 0) {
        callback(ec, std::any{});
        return;
    }
    callback(ec, std::make_any<std::tuple<Args&&...>>(std::forward_as_tuple(std::forward<Args>(args)...)));
}

} // namespace oink_judge::socket