#pragma once

#include "socket/Session.hpp"
#include "socket/SessionEventHandler.hpp"
#include <boost/asio/ssl.hpp>

namespace oink_judge::socket {

class BlockingSSLSession : public Session, public std::enable_shared_from_this<Session> {
public:
    BlockingSSLSession(tcp::socket socket, std::unique_ptr<SessionEventHandler> event_handler, boost::asio::ssl::stream_base::handshake_type _handshake_type);
    ~BlockingSSLSession() override;

    void set_session_ptr();
    void start(const std::string &start_message) override;
    void send_message(const std::string &message) override;
    void receive_message() override;
    void close() override;

    constexpr static auto REGISTERED_NAME_SERVER = "BlockingSSLSessionServer";
    constexpr static auto REGISTERED_NAME_CLIENT = "BlockingSSLSessionClient";

private:
    boost::asio::ssl::stream<tcp::socket> _ssl_stream;
    std::unique_ptr<SessionEventHandler> _event_handler;
    boost::asio::ssl::stream_base::handshake_type _handshake_type;
};

} // namespace oink_judge::soket::sessions
