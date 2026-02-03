#include "socket/sessions/SSLSession.h"
#include "socket/BoostSSLContext.h"
#include "socket/ConnectionStorage.h"
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

SSLSession::SSLSession(tcp::socket socket, std::unique_ptr<Protocol> protocol,  boost::asio::ssl::stream_base::handshake_type handshake_type)
    : SessionBase(std::move(protocol)),
      _is_sending(false),
      _ssl_stream(std::move(socket), (handshake_type == boost::asio::ssl::stream_base::server) ? BoostSSLContext::server() : BoostSSLContext::client()),
      _handshake_type(handshake_type) {}

SSLSession::~SSLSession() {
    close();
    // ConnectionStorage::instance().remove_connection(std::static_pointer_cast<SSLSession>(shared_from_this()));
}

awaitable<void> SSLSession::start(const std::string &start_message) {
    if (!_ssl_stream.lowest_layer().is_open()) {
        throw std::runtime_error("SSL socket is not open.");
    }

    // keep session alive for duration of async operations
    auto self = std::static_pointer_cast<SSLSession>(shared_from_this());

    ConnectionStorage::instance().insert_connection(self);

    try {
        _ssl_stream.set_verify_mode(boost::asio::ssl::verify_peer);

        std::cout << "wait hanshake" << std::endl;

        co_await _ssl_stream.async_handshake(_handshake_type, boost::asio::use_awaitable);

        std::cout << "Handshake" << _handshake_type << std::endl;
        co_await access_protocol().start(start_message);

    } catch (const boost::system::system_error& e) {
        std::cout << "Error " << e.what() << std::endl;
        close();
    }
}

awaitable<void> SSLSession::send_message(const std::string &message) {
    std::cerr << "Queueing message to send: " << message << std::endl;
    // keep session alive for duration of async operations
    auto self = std::static_pointer_cast<SSLSession>(shared_from_this());

    co_return co_await boost::asio::async_initiate<decltype(boost::asio::use_awaitable), void(boost::system::error_code)>(
        [self, message](auto&& handler) {
            self->_message_queue.push({message, std::move(handler)});

            if (!self->_is_sending) {
                self->_is_sending = true;
                boost::asio::co_spawn(self->_ssl_stream.get_executor(), [self]() -> awaitable<void> { co_await self->_send_loop(); }, boost::asio::detached);
            }
        },
        boost::asio::use_awaitable
    );
}

awaitable<void> SSLSession::receive_message() {
    // keep session alive for duration of async operations
    auto self = std::static_pointer_cast<SSLSession>(shared_from_this());

    try {
        std::cout << "Receiving message" << std::endl;

        uint64_t message_length_net = 0;

        co_await boost::asio::async_read(
            _ssl_stream,
            boost::asio::buffer(&message_length_net, sizeof message_length_net),
            boost::asio::use_awaitable
        );

        uint64_t message_length = ntoh64(message_length_net);
        std::cout << "Reading message of size " << message_length << std::endl;

        std::string message(static_cast<size_t>(message_length), '\0');

        co_await boost::asio::async_read(
            _ssl_stream,
            boost::asio::buffer(message),
            boost::asio::use_awaitable
        );

        std::cout << "Read message: " << message << std::endl;
        co_await access_protocol().receive_message(message);
    } catch (const boost::system::system_error& e) {
        std::cout << "Receive error: " << e.what() << std::endl;
        close();
    }
}

void SSLSession::close() {
    if (_ssl_stream.lowest_layer().is_open()) {
        _ssl_stream.lowest_layer().close();
        access_protocol().close_session();
    }
}

boost::asio::any_io_executor SSLSession::get_executor() {
    return _ssl_stream.get_executor();
}

awaitable<void> SSLSession::_send_loop () {
    // keep session alive for duration of async operations
    auto self = std::static_pointer_cast<SSLSession>(shared_from_this());
    
    while (!_message_queue.empty()) {
        QueuedMessage qm = std::move(_message_queue.front());
        _message_queue.pop();

        std::cerr << "Sending queued message: " << qm.message << std::endl;

        uint64_t message_length = hton64(qm.message.size());

        boost::system::error_code ec;

        co_await boost::asio::async_write(
            _ssl_stream,
            boost::asio::buffer(&message_length, sizeof message_length),
            boost::asio::redirect_error(boost::asio::use_awaitable, ec)
        );

        if (ec) {
            qm.callback(ec);
            close();
            co_return;
        }

        std::cerr << "Sent message length: " << ntoh64(message_length) << std::endl;

        co_await boost::asio::async_write(
            _ssl_stream,
            boost::asio::buffer(qm.message),
            boost::asio::redirect_error(boost::asio::use_awaitable, ec)
        );

        if (ec) {
            qm.callback(ec);
            close();
            co_return;
        }

        std::cerr << "Sent message body: " << qm.message << std::endl;

        qm.callback(ec);
    }

    _is_sending = false;
}

} // namespace oink_judge::socket
