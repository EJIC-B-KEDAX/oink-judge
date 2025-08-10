#pragma once

#include <boost/asio.hpp>
#include <socket/Session.hpp>

namespace oink_judge::socket {

using tcp = boost::asio::ip::tcp;

void async_connect_to_the_endpoint(const std::string &host, short port, const std::string &session_type, const std::string &request);

void async_schedule_connect_to_the_endpoint(const std::string &host, short port, const std::string &session_type, const std::string &request);

std::shared_ptr<Session> connect_to_the_endpoint(const std::string &host, short port, const std::string &session_type, const std::string &request);

std::shared_ptr<Session> schedule_connect_to_the_endpoint(const std::string &host, short port, const std::string &session_type, const std::string &request);

} // namespace oink_judge::socket
