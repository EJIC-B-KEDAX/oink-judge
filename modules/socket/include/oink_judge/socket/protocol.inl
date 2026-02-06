#pragma once
#include "oink_judge/socket/protocol.hpp"

namespace oink_judge::socket {

template <typename... Args>
void Protocol::callCallback(const callback_t& callback, std::error_code ec, std::decay_t<Args>... args) {
    if constexpr (sizeof...(Args) == 0) {
        callback(ec, std::any{});
        return;
    }

    std::any packed_args = std::make_any<std::tuple<std::decay_t<Args>...>>(std::tuple<std::decay_t<Args>...>(args...));
    callback(ec, packed_args);
}

} // namespace oink_judge::socket
