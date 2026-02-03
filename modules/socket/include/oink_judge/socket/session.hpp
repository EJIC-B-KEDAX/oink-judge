#pragma once

#include <boost/asio.hpp>
#include "ParameterizedTypeFactory.hpp"

namespace oink_judge::socket {

using boost::asio::ip::tcp;
using boost::asio::awaitable;

template<typename... Args>
using awaitable_packed_types_t = boost::asio::async_result<
    boost::asio::use_awaitable_t<>,
    void(std::decay_t<Args>...)>
    ::return_type;

class Session {
public:
    using callback_t = std::function<void(std::error_code, std::any)>;
    
    virtual ~Session() = default;

    virtual awaitable<void> start(const std::string &start_message) = 0;
    virtual awaitable<void> send_message(const std::string &message) = 0;
    virtual awaitable<void> receive_message() = 0;
    virtual void close() = 0;

    template<typename... Args>
    awaitable_packed_types_t<Args...> request(const std::string &message);
    template<typename F, typename... Args>
    void request(const std::string &message, F&& callback);

    virtual void request_internal(const std::string &message, const callback_t &callback) = 0;

    virtual boost::asio::any_io_executor get_executor() = 0;
};

using SessionFactory = ParameterizedTypeFactory<std::shared_ptr<Session>, tcp::socket>;

} // namespace oink_judge::socket

#include "Session.inl"
