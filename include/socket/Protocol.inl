#pragma once
#include "Protocol.hpp"

namespace oink_judge::socket {

template<typename F, typename... Args>
void Protocol::request(const std::string &message, F&& callback) {
    static_assert(std::is_invocable_v<F, std::error_code, Args...>,
                      "Callback signature must be: void(std::error_code, Args...)");

    auto wrapped_callback = [cb = std::forward<F>(callback)](std::error_code ec, std::any result) mutable {
        if constexpr (sizeof...(Args) == 0) {
            cb(ec);
            return;
        } else if constexpr (sizeof...(Args) == 1) {
            using T = std::tuple_element_t<0, std::tuple<Args&&...>>;
            cb(ec, std::any_cast<T>(result));
        } else {
            using TupleType = std::tuple<Args&&...>;
            std::apply(
                [ec, &cb](Args&&... unpacked_args) mutable {
                    cb(ec, std::forward<Args>(unpacked_args)...);
                },
                std::any_cast<TupleType>(result)
            );
        }
    };
    
    request_internal(message, wrapped_callback);
}

template<typename... Args>
void Protocol::call_callback(const callback_t &callback, std::error_code ec, Args&&... args) {
    if constexpr (sizeof...(Args) == 0) {
        callback(ec, std::any{});
        return;
    }
    callback(ec, std::make_any<std::tuple<Args&&...>>(std::forward_as_tuple(std::forward<Args>(args)...)));
}

} // namespace oink_judge::socket