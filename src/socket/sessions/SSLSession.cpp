#include "socket/sessions/SSLSession.h"
#include "socket/BoostSSLContext.h"
#include "socket/byte_order.h"

namespace oink_judge::socket {

namespace {

[[maybe_unused]] bool registered_client = []() -> bool {
    BasicSessionFactory::instance().register_type(
        SSLSession::REGISTERED_NAME_CLIENT,
        [](const std::string &params, tcp::socket socket) -> std::shared_ptr<Session> {
        auto event_handler = BasicSessionEventHandlerFactory::instance().create(params);

        return std::make_shared<SSLSession>(
            std::move(socket),
            std::move(event_handler),
            boost::asio::ssl::stream_base::client);
    });

    return true;
}();

[[maybe_unused]] bool registered_server = []() -> bool {
    BasicSessionFactory::instance().register_type(
        SSLSession::REGISTERED_NAME_SERVER,
        [](const std::string &params, tcp::socket socket) -> std::shared_ptr<Session> {
        auto event_handler = BasicSessionEventHandlerFactory::instance().create(params);

        return std::make_shared<SSLSession>(
            std::move(socket),
            std::move(event_handler),
            boost::asio::ssl::stream_base::server);
    });

    return true;
}();

} // namespace

SSLSession::SSLSession(tcp::socket socket, std::unique_ptr<SessionEventHandler> event_handler,  boost::asio::ssl::stream_base::handshake_type handshake_type)
    : _ssl_stream(std::move(socket), BoostSSLContext::instance()), 
      _event_handler(std::move(event_handler)),
      _handshake_type(handshake_type) {

    if (_event_handler) {
        _event_handler->set_session(shared_from_this());
    }
}

SSLSession::~SSLSession() {
    close();
}

void SSLSession::start(const std::string &start_message) {
    if (!_ssl_stream.lowest_layer().is_open()) {
        throw std::runtime_error("SSL socket is not open.");
    }

    auto self = shared_from_this();

    _ssl_stream.set_verify_mode(boost::asio::ssl::verify_peer);

    _ssl_stream.async_handshake(_handshake_type,
        [self, this, start_message](const boost::system::error_code &error) {
        if (!error) {
            _event_handler->start(start_message);
        } else {
            close();
        }
    });
}

void SSLSession::send_message(const std::string &message) {
    _message_queue.push(message);

    if (_message_queue.size() == 1) {
        _send_next();
    }
}

void SSLSession::receive_message() {
    auto self = shared_from_this();

    size_t message_length_net;
    boost::asio::async_read(_ssl_stream, boost::asio::buffer(&message_length_net, sizeof(message_length_net)),
        [self, this, message_length_net](const boost::system::error_code &ec, std::size_t /*length*/) {
        if (!ec) {
            size_t message_length = ntoh64(message_length_net);
            std::string message(message_length, '\0');

            boost::asio::async_read(_ssl_stream, boost::asio::buffer(message),
                [self, this, message](const boost::system::error_code &ec, std::size_t /*length*/) {
                if (!ec) {
                    _event_handler->receive_message(message);
                } else {
                    close();
                }
            });
        } else {
            close();
        }
    });
}

void SSLSession::close() {
    if (_ssl_stream.lowest_layer().is_open()) {
        _ssl_stream.lowest_layer().close();
        _event_handler->close_session();
    }
}

void SSLSession::_send_next() {
    if (_message_queue.empty()) return;

    std::string message = std::move(_message_queue.front());

    auto self = shared_from_this();

    size_t message_length = hton64(message.size());
    boost::asio::async_write(_ssl_stream, boost::asio::buffer(&message_length, sizeof(message_length)),
        [self, this, message](const boost::system::error_code &ec, std::size_t /*length*/) {
        if (!ec) {
            boost::asio::async_write(_ssl_stream, boost::asio::buffer(message),
                [self, this, message](const boost::system::error_code &ec, std::size_t /*length*/) {
                if (!ec) {
                    _message_queue.pop();
                    _send_next();
                } else {
                    close();
                }
            });
        } else {
            close();
        }
    });
}

} // namespace oink_judge::socket
