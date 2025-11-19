#include "socket/sessions/SSLSession.h"
#include "socket/BoostSSLContext.h"
#include "socket/byte_order.h"
#include <iostream>

namespace oink_judge::socket {

namespace {

[[maybe_unused]] bool registered_client = []() -> bool {
    SessionFactory::instance().register_type(
        SSLSession::REGISTERED_NAME_CLIENT,
        [](const std::string &params, tcp::socket socket) -> std::shared_ptr<Session> {
        auto event_handler = ProtocolFactory::instance().create(params);

        auto ptr = std::make_shared<SSLSession>(
            std::move(socket),
            std::move(event_handler),
            boost::asio::ssl::stream_base::client);

        ptr->set_session_ptr();

        return ptr;
    });

    return true;
}();

[[maybe_unused]] bool registered_server = []() -> bool {
    SessionFactory::instance().register_type(
        SSLSession::REGISTERED_NAME_SERVER,
        [](const std::string &params, tcp::socket socket) -> std::shared_ptr<Session> {
        std::cout << "Creating SSL server session with params: " << params << std::endl;
        auto event_handler = ProtocolFactory::instance().create(params);

        auto ptr = std::make_shared<SSLSession>(
            std::move(socket),
            std::move(event_handler),
            boost::asio::ssl::stream_base::server);

        ptr->set_session_ptr();

        return ptr;
    });

    return true;
}();

} // namespace

SSLSession::SSLSession(tcp::socket socket, std::unique_ptr<Protocol> event_handler,  boost::asio::ssl::stream_base::handshake_type handshake_type)
    : _ssl_stream(std::move(socket), (handshake_type == boost::asio::ssl::stream_base::server) ? BoostSSLContext::server() : BoostSSLContext::client()), 
      _protocol(std::move(event_handler)),
      _handshake_type(handshake_type) {

    std::cout << "Created SSL session" << std::endl;
}

SSLSession::~SSLSession() {
    close();
}

void SSLSession::set_session_ptr() {
    _protocol->set_session(shared_from_this());
}

void SSLSession::start(const std::string &start_message) {
    if (!_ssl_stream.lowest_layer().is_open()) {
        throw std::runtime_error("SSL socket is not open.");
    }

    auto self = shared_from_this();

    _ssl_stream.set_verify_mode(boost::asio::ssl::verify_peer);

    std::cout << "wait hanshake" << std::endl;

    _ssl_stream.async_handshake(_handshake_type,
        [self, this, start_message](const boost::system::error_code &error) {
        std::cout << "Handshake" << std::endl;
        std::cout << _handshake_type << std::endl;
        if (!error) {
            _protocol->start(start_message);
        } else {
            std::cout << "Error " << error.what() << std::endl;
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
    std::cout << "Receiving message" << std::endl;
    auto self = shared_from_this();

    auto message_length_net_ptr = std::make_shared<std::array<char, sizeof(size_t)>>();
    boost::asio::async_read(_ssl_stream, boost::asio::buffer(*message_length_net_ptr),
        [self, this, message_length_net_ptr](const boost::system::error_code &ec, std::size_t /*length*/) {
        if (!ec) {
            size_t message_length_net = 0;
            std::memcpy(&message_length_net, message_length_net_ptr->data(), sizeof(message_length_net));
            size_t message_length = ntoh64(message_length_net);
            std::cout << "Reading message of size " << message_length << std::endl;
            auto message_ptr = std::make_shared<std::string>(message_length, '\0');

            boost::asio::async_read(_ssl_stream, boost::asio::buffer(*message_ptr),
                [self, this, message_ptr](const boost::system::error_code &ec, std::size_t /*length*/) {
                if (!ec) {
                    std::cout << "Read message: " << *message_ptr << std::endl;
                    _protocol->receive_message(*message_ptr);
                } else {
                    std::cout << "Error reading message: " << ec.what() << std::endl;
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
        _protocol->close_session();
    }
}

void SSLSession::_send_next() {
    if (_message_queue.empty()) return;

    std::string message = std::move(_message_queue.front());
    auto message_ptr = std::make_shared<std::string>(message);

    if (message_ptr->size() <= 200) {
        std::cout << "Sending message: " << *message_ptr << std::endl;
    }

    auto self = shared_from_this();

    size_t message_length = hton64(message.size());
    auto message_length_net_ptr = std::make_shared<std::array<char, sizeof(message_length)>>();
    std::memcpy(message_length_net_ptr->data(), &message_length, sizeof(message_length));
    boost::asio::async_write(_ssl_stream, boost::asio::buffer(*message_length_net_ptr),
        [self, this, message_ptr, message_length_net_ptr](const boost::system::error_code &ec, std::size_t /*length*/) {
        if (!ec) {
            std::cout << "Sent length of the message: " << message_ptr->size() << std::endl;
            boost::asio::async_write(_ssl_stream, boost::asio::buffer(*message_ptr),
                [self, this, message_ptr](const boost::system::error_code &ec, std::size_t /*length*/) {
                if (!ec) {
                    if (message_ptr->size() < 200) {
                        std::cout << "Sent message: " << *message_ptr << std::endl;
                    }
                    _message_queue.pop();
                    _send_next();
                } else {
                    std::cout << "Error sending message: " << ec.what() << std::endl;
                    close();
                }
            });
        } else {
            close();
        }
    });
}

} // namespace oink_judge::socket
