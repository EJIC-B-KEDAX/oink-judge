#include "oink_judge/socket/sessions/ssl_session.h"

#include "oink_judge/socket/boost_ssl_context.h"
#include "oink_judge/socket/byte_order.h"
#include "oink_judge/socket/connection_storage.h"

#include <oink_judge/logger/logger.h>

namespace oink_judge::socket {

namespace {

[[maybe_unused]] const bool REGISTERED_CLIENT = []() -> bool {
    SessionFactory::instance().registerType(
        SSLSession::REGISTERED_NAME_CLIENT, [](const std::string& params, tcp::socket socket) -> std::shared_ptr<Session> {
            auto event_handler = ProtocolFactory::instance().create(params);

            auto ptr =
                std::make_shared<SSLSession>(std::move(socket), std::move(event_handler), boost::asio::ssl::stream_base::client);

            ptr->setSessionPtr();

            return ptr;
        });

    return true;
}();

[[maybe_unused]] const bool REGISTERED_SERVER = []() -> bool {
    SessionFactory::instance().registerType(
        SSLSession::REGISTERED_NAME_SERVER, [](const std::string& params, tcp::socket socket) -> std::shared_ptr<Session> {
            auto event_handler = ProtocolFactory::instance().create(params);

            auto ptr =
                std::make_shared<SSLSession>(std::move(socket), std::move(event_handler), boost::asio::ssl::stream_base::server);

            ptr->setSessionPtr();

            return ptr;
        });

    return true;
}();

} // namespace

SSLSession::SSLSession(tcp::socket socket, std::unique_ptr<Protocol> protocol,
                       boost::asio::ssl::stream_base::handshake_type handshake_type)
    : SessionBase(std::move(protocol)), is_sending_(false),
      ssl_stream_(std::move(socket), (handshake_type == boost::asio::ssl::stream_base::server) ? BoostSSLContext::server()
                                                                                               : BoostSSLContext::client()),
      handshake_type_(handshake_type) {}

SSLSession::~SSLSession() {
    close();
    // ConnectionStorage::instance().remove_connection(std::static_pointer_cast<SSLSession>(shared_from_this()));
}

auto SSLSession::start(std::string start_message) -> awaitable<void> {
    if (!ssl_stream_.lowest_layer().is_open()) {
        throw std::runtime_error("SSL socket is not open.");
    }

    // keep session alive for duration of async operations
    auto self = std::static_pointer_cast<SSLSession>(shared_from_this());

    ConnectionStorage::instance().insertConnection(self);

    try {
        ssl_stream_.set_verify_mode(boost::asio::ssl::verify_peer);

        co_await ssl_stream_.async_handshake(handshake_type_, boost::asio::use_awaitable);
        co_await accessProtocol().start(start_message);

    } catch (const boost::system::system_error& e) {
        logger::logMessage("SSLSession", 1, std::string("Error: ") + e.what(), logger::ERROR);
        close();
    }
}

auto SSLSession::sendMessage(std::string message) -> awaitable<void> {
    // keep session alive for duration of async operations
    auto self = std::static_pointer_cast<SSLSession>(shared_from_this());

    co_return co_await boost::asio::async_initiate<decltype(boost::asio::use_awaitable), void(boost::system::error_code)>(
        [self, message](auto&& handler) -> auto {
            self->message_queue_.push({message, std::forward<decltype(handler)>(handler)});

            if (!self->is_sending_) {
                self->is_sending_ = true;
                boost::asio::co_spawn(
                    self->ssl_stream_.get_executor(), [self]() -> awaitable<void> { co_await self->sendLoop(); }, // NOLINT
                    boost::asio::detached);
            }
        },
        boost::asio::use_awaitable);
}

auto SSLSession::receiveMessage() -> awaitable<void> {
    // keep session alive for duration of async operations
    auto self = std::static_pointer_cast<SSLSession>(shared_from_this());

    try {
        uint64_t message_length_net = 0;

        co_await boost::asio::async_read(ssl_stream_, boost::asio::buffer(&message_length_net, sizeof message_length_net),
                                         boost::asio::use_awaitable);

        uint64_t message_length = ntoh64(message_length_net);

        std::string message(static_cast<size_t>(message_length), '\0');

        co_await boost::asio::async_read(ssl_stream_, boost::asio::buffer(message), boost::asio::use_awaitable);
        co_await accessProtocol().receiveMessage(message);
    } catch (const boost::system::system_error& e) {
        logger::logMessage("SSLSession", 1, std::string("Receive error: ") + e.what(), logger::ERROR);
        close();
    }
}

auto SSLSession::close() -> void {
    if (ssl_stream_.lowest_layer().is_open()) {
        ssl_stream_.lowest_layer().close();
        accessProtocol().closeSession();
    }
}

auto SSLSession::getExecutor() -> boost::asio::any_io_executor { return ssl_stream_.get_executor(); }

auto SSLSession::sendLoop() -> awaitable<void> {
    // keep session alive for duration of async operations
    auto self = std::static_pointer_cast<SSLSession>(shared_from_this());

    while (!message_queue_.empty()) {
        QueuedMessage qm = std::move(message_queue_.front());
        message_queue_.pop();

        uint64_t message_length = hton64(qm.message.size());

        boost::system::error_code ec;

        co_await boost::asio::async_write(ssl_stream_, boost::asio::buffer(&message_length, sizeof message_length),
                                          boost::asio::redirect_error(boost::asio::use_awaitable, ec));

        if (ec) {
            logger::logMessage("SSLSession", 1, std::string("Send length error: ") + ec.message(), logger::ERROR);
            qm.callback(ec);
            close();
            co_return;
        }

        co_await boost::asio::async_write(ssl_stream_, boost::asio::buffer(qm.message),
                                          boost::asio::redirect_error(boost::asio::use_awaitable, ec));

        if (ec) {
            logger::logMessage("SSLSession", 1, std::string("Send message error: ") + ec.message(), logger::ERROR);
            qm.callback(ec);
            close();
            co_return;
        }

        qm.callback(ec);
    }

    is_sending_ = false;
}

} // namespace oink_judge::socket
