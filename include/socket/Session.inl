#pragma once

#include "Session.hpp"

namespace oink_judge::socket {

template<typename F, typename... Args>
void Session::request(const std::string &message, F&& callback) {
    static_assert(std::is_invocable_v<F, std::error_code, Args...>,
                      "Callback signature must be: void(std::error_code, Args...)");

    auto wrapped_callback = [cb = std::forward<F>(callback)](std::error_code ec, std::any result) mutable {
        if constexpr (sizeof...(Args) == 0) {
            cb(ec);
            return;
        }
        
        using TupleType = std::tuple<Args&&...>;
        std::apply(
            [ec, &cb](Args&&... unpacked_args) mutable {
                cb(ec, std::forward<Args>(unpacked_args)...);
            },
            std::any_cast<TupleType>(result)
        );
    };
    
    request_internal(message, wrapped_callback);
}

} // namespace oink_judge::socket
