#pragma once

#include <boost/asio.hpp>
#include <socket/Session.hpp>

namespace oink_judge::socket {

using tcp = boost::asio::ip::tcp;

awaitable<std::shared_ptr<Session>> async_connect_to_the_endpoint(std::string host, short port, std::string session_type, std::string request);

awaitable<std::shared_ptr<Session>> async_schedule_connect_to_the_endpoint(std::string host, short port, std::string session_type, std::string request);

std::shared_ptr<Session> connect_to_the_endpoint(std::string host, short port, std::string session_type, std::string request);

std::shared_ptr<Session> schedule_connect_to_the_endpoint(std::string host, short port, std::string session_type, std::string request);

} // namespace oink_judge::socket
