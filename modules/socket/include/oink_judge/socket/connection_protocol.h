#pragma once
#include "oink_judge/socket/session.hpp"

#include <boost/asio.hpp>

namespace oink_judge::socket {

using tcp = boost::asio::ip::tcp;

auto asyncConnectToTheEndpoint(std::string host, short port, std::string session_type, std::string request)
    -> awaitable<std::shared_ptr<Session>>;

auto asyncScheduleConnectToTheEndpoint(std::string host, short port, std::string session_type, std::string request)
    -> awaitable<std::shared_ptr<Session>>;

auto connectToTheEndpoint(std::string host, short port, std::string session_type, std::string request)
    -> std::shared_ptr<Session>;

auto scheduleConnectToTheEndpoint(std::string host, short port, std::string session_type, std::string request)
    -> std::shared_ptr<Session>;

} // namespace oink_judge::socket
