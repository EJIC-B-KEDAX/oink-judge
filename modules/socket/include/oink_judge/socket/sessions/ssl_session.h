#pragma once
#include "oink_judge/socket/protocol.hpp"
#include "oink_judge/socket/sessions/session_base.h"

#include <boost/asio/ssl.hpp>
#include <queue>

namespace oink_judge::socket {

using tcp = boost::asio::ip::tcp;

class SSLSession : public SessionBase {
  public:
    SSLSession(tcp::socket socket, std::unique_ptr<Protocol> protocol,
               boost::asio::ssl::stream_base::handshake_type _handshake_type);

    SSLSession(const SSLSession&) = delete;
    auto operator=(const SSLSession&) -> SSLSession& = delete;
    SSLSession(SSLSession&&) = delete;
    auto operator=(SSLSession&&) -> SSLSession& = delete;
    ~SSLSession() override;

    auto start(std::string start_message) -> awaitable<void> override;
    auto sendMessage(std::string message) -> awaitable<void> override;
    auto receiveMessage() -> awaitable<void> override;
    auto close() -> void override;

    auto getExecutor() -> boost::asio::any_io_executor override;

    constexpr static auto REGISTERED_NAME_SERVER = "SSLSessionServer";
    constexpr static auto REGISTERED_NAME_CLIENT = "SSLSessionClient";

  private:
    boost::asio::ssl::stream<tcp::socket> ssl_stream_;
    boost::asio::ssl::stream_base::handshake_type handshake_type_;

    struct QueuedMessage {
        std::string message;
        std::move_only_function<void(boost::system::error_code)> callback;
    };
    std::queue<QueuedMessage> message_queue_;

    auto sendLoop() -> awaitable<void>;
    bool is_sending_;
};

} // namespace oink_judge::socket
