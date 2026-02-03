#pragma once

#include "Session.hpp"
#include <iostream>

namespace oink_judge::socket {

template<typename... Args>
awaitable_packed_types_t<Args...> Session::request(const std::string &message) {
    co_return co_await boost::asio::async_initiate<
        decltype(boost::asio::use_awaitable),
        void(boost::system::error_code, std::decay_t<Args>...)>(
        [this, message](auto handler) {
            auto wrapped_handler = [handler = std::move(handler)](boost::system::error_code ec, std::any result) mutable {
                if constexpr (sizeof...(Args) == 0) {
                    handler(ec);
                } else {
                    using TupleType = std::tuple<std::decay_t<Args>...>;
                    TupleType tup = std::any_cast<TupleType>(result);
                    std::apply(
                        [ec, &handler](auto&&... unpacked_args) mutable {
                            handler(ec, std::forward<decltype(unpacked_args)>(unpacked_args)...);
                        },
                        std::move(tup)
                    );
                }
            };
            
            auto handler_ptr = std::make_shared<decltype(wrapped_handler)>(std::move(wrapped_handler));
            request_internal(message, [handler_ptr](std::error_code ec, std::any result) {
                (*handler_ptr)(ec, result);
            });
        },
        boost::asio::use_awaitable
    );
}

template<typename F, typename... Args>
void Session::request(const std::string &message, F&& callback) {
    static_assert(std::is_invocable_v<F, std::error_code, std::decay_t<Args>...>,
                      "Callback signature must be: void(std::error_code, Args...)");

    auto wrapped_callback = [cb = std::forward<F>(callback)](std::error_code ec, std::any result) mutable {
        if constexpr (sizeof...(Args) == 0) {
            cb(ec);
            return;
        }
        
        using TupleType = std::tuple<std::decay_t<Args>...>;
        TupleType tup = std::any_cast<TupleType>(result);
        std::apply(
            [ec, &cb](auto&&... unpacked_args) mutable {
                cb(ec, std::forward<decltype(unpacked_args)>(unpacked_args)...);
            },
            std::move(tup)
        );
    };
    
    request_internal(message, wrapped_callback);
}

} // namespace oink_judge::socket
