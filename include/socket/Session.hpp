#pragma once

#include <boost/asio.hpp>
#include "ParameterizedTypeFactory.hpp"

namespace oink_judge::socket {

using tcp = boost::asio::ip::tcp;

class Session {
public:
    virtual ~Session() = default;

    virtual void start(const std::string &start_message) = 0;
    virtual void send_message(const std::string &message) = 0;
    virtual void receive_message() = 0;
    virtual void close() = 0;
};

using SessionFactory = ParameterizedTypeFactory<std::shared_ptr<Session>, tcp::socket>;

} // namespace oink_judge::socket
