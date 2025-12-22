#pragma once

#include <boost/asio.hpp>
#include "ParameterizedTypeFactory.hpp"

namespace oink_judge::socket {

using tcp = boost::asio::ip::tcp;

class Session {
public:
    using callback_t = std::function<void(std::error_code, std::any)>;
    
    virtual ~Session() = default;

    virtual void start(const std::string &start_message) = 0;
    virtual void send_message(const std::string &message) = 0;
    virtual void receive_message() = 0;
    virtual void close() = 0;

    template<typename F, typename... Args>
    void request(const std::string &message, F&& callback);

    virtual void request_internal(const std::string &message, const callback_t &callback) = 0;
};

using SessionFactory = ParameterizedTypeFactory<std::shared_ptr<Session>, tcp::socket>;

} // namespace oink_judge::socket

#include "Session.inl"
