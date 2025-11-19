#pragma once

#include "socket/Session.hpp"
#include "socket/Protocol.hpp"
#include <boost/asio/ssl.hpp>
#include <queue>

namespace oink_judge::socket {

using tcp = boost::asio::ip::tcp;

class SSLSession : public Session, public std::enable_shared_from_this<Session> {
public:
    SSLSession(tcp::socket socket, std::unique_ptr<Protocol> event_handler, boost::asio::ssl::stream_base::handshake_type _handshake_type);
    ~SSLSession() override;

    void set_session_ptr();
    void start(const std::string &start_message) override;
    void send_message(const std::string &message) override;
    void receive_message() override;
    void close() override;

    constexpr static auto REGISTERED_NAME_SERVER = "SSLSessionServer";
    constexpr static auto REGISTERED_NAME_CLIENT = "SSLSessionClient";

private:
    void _send_next();

    boost::asio::ssl::stream<tcp::socket> _ssl_stream;
    std::unique_ptr<Protocol> _protocol;
    std::queue<std::string> _message_queue;
    boost::asio::ssl::stream_base::handshake_type _handshake_type;
};

} // namespace oink_judge::socket
