#pragma once

#include "socket/sessions/SessionBase.h"
#include "socket/Protocol.hpp"
#include <boost/asio/ssl.hpp>
#include <queue>

namespace oink_judge::socket {

using tcp = boost::asio::ip::tcp;

class SSLSession : public SessionBase {
public:
    SSLSession(tcp::socket socket, std::unique_ptr<Protocol> protocol, boost::asio::ssl::stream_base::handshake_type _handshake_type);
    ~SSLSession() override;

    awaitable<void> start(const std::string &start_message) override;
    awaitable<void> send_message(const std::string &message) override;
    awaitable<void> receive_message() override;
    void close() override;

    boost::asio::any_io_executor get_executor();

    constexpr static auto REGISTERED_NAME_SERVER = "SSLSessionServer";
    constexpr static auto REGISTERED_NAME_CLIENT = "SSLSessionClient";

private:
    boost::asio::ssl::stream<tcp::socket> _ssl_stream;
    boost::asio::ssl::stream_base::handshake_type _handshake_type;

    struct QueuedMessage {
        std::string message;
        std::move_only_function<void(boost::system::error_code)> callback;
    };
    std::queue<QueuedMessage> _message_queue;

    awaitable<void> _send_loop();
    bool _is_sending;
};

} // namespace oink_judge::socket
