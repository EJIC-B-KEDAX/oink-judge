#include "socket/sessions/BlockingSSLSession.h"
#include "socket/BoostSSLContext.h"
#include "socket/byte_order.h"

namespace oink_judge::socket {

namespace {

[[maybe_unused]] bool registered_client = []() -> bool {
    BasicSessionFactory::instance().register_type(
        BlockingSSLSession::REGISTERED_NAME_CLIENT,
        [](const std::string &params, tcp::socket socket) -> std::shared_ptr<Session> {
        auto event_handler = BasicSessionEventHandlerFactory::instance().create(params);

        auto ptr = std::make_shared<BlockingSSLSession>(
            std::move(socket),
            std::move(event_handler),
            boost::asio::ssl::stream_base::client);

        ptr->set_session_ptr();

        return ptr;
    });

    return true;
}();

[[maybe_unused]] bool registered_server = []() -> bool {
    BasicSessionFactory::instance().register_type(
        BlockingSSLSession::REGISTERED_NAME_SERVER,
        [](const std::string &params, tcp::socket socket) -> std::shared_ptr<Session> {
        auto event_handler = BasicSessionEventHandlerFactory::instance().create(params);

        auto ptr = std::make_shared<BlockingSSLSession>(
            std::move(socket),
            std::move(event_handler),
            boost::asio::ssl::stream_base::server);

        ptr->set_session_ptr();

        return ptr;
    });

    return true;
}();

} // namespace


BlockingSSLSession::BlockingSSLSession(tcp::socket socket, std::unique_ptr<SessionEventHandler> event_handler,  boost::asio::ssl::stream_base::handshake_type handshake_type)
    : _ssl_stream(std::move(socket), (handshake_type == boost::asio::ssl::stream_base::handshake_type::server) ? BoostSSLContext::server() : BoostSSLContext::client()), 
      _event_handler(std::move(event_handler)),
      _handshake_type(handshake_type) {
}

BlockingSSLSession::~BlockingSSLSession() {
    close();
}

void BlockingSSLSession::set_session_ptr() {
    _event_handler->set_session(shared_from_this());
}

void BlockingSSLSession::start(const std::string &start_message) {
    if (!_ssl_stream.lowest_layer().is_open()) {
        throw std::runtime_error("SSL socket is not open.");
    }

    auto self = shared_from_this();

    _ssl_stream.handshake(_handshake_type);
    _event_handler->start(start_message);
}

void BlockingSSLSession::send_message(const std::string &message) {
    if (!_ssl_stream.lowest_layer().is_open()) {
        throw std::runtime_error("SSL socket is not open.");
    }

    size_t message_length_net = hton64(message.size());

    boost::asio::write(_ssl_stream, boost::asio::buffer(&message_length_net, sizeof(message_length_net)));
    boost::asio::write(_ssl_stream, boost::asio::buffer(message));
}

void BlockingSSLSession::receive_message() {
    size_t message_length_net;
    boost::asio::read(_ssl_stream, boost::asio::buffer(&message_length_net, sizeof(message_length_net)));
    size_t message_length = ntoh64(message_length_net);

    std::string message(message_length, '\0');
    boost::asio::read(_ssl_stream, boost::asio::buffer(message));
    _event_handler->receive_message(message);
}

void BlockingSSLSession::close() {
    if (_ssl_stream.lowest_layer().is_open()) {
        _ssl_stream.lowest_layer().close();
        _event_handler->close_session();
    }
}

} // namespace oink_judge::socket
