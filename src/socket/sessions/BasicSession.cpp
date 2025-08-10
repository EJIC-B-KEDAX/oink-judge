#include "socket/sessions/BasicSession.h"
#include "socket/byte_order.h"

namespace oink_judge::socket {

namespace {

[[maybe_unused]] bool registered = []() -> bool {
    BasicSessionFactory::instance().register_type(BasicSession::REGISTERED_NAME, [](const std::string &params, tcp::socket socket) -> std::shared_ptr<Session> {
        auto event_handler = BasicSessionEventHandlerFactory::instance().create(params);

        return std::make_shared<BasicSession>(std::move(socket), std::move(event_handler));
    });

    return true;
}();

} // namespace

BasicSession::BasicSession(tcp::socket socket, std::unique_ptr<SessionEventHandler> event_handler) :
    SessionBase(std::move(socket), std::move(event_handler)) {}

void BasicSession::start(const std::string &start_message) {
    access_event_handler().start(start_message);
}

void BasicSession::send_message(const std::string &message) {
    _message_queue.push(message);

    if (_message_queue.size() == 1) {
        _send_next();
    }
}

void BasicSession::receive_message() {
    auto self = shared_from_this();

    size_t message_length_net;
    boost::asio::async_read(access_socket(), boost::asio::buffer(&message_length_net, sizeof(message_length_net)),
        [self, this, message_length_net](const boost::system::error_code &ec, std::size_t /*length*/) -> void {
        if (!ec) {
            size_t message_length = ntoh64(message_length_net);
            std::string message(message_length, '\0');

            boost::asio::async_read(access_socket(), boost::asio::buffer(message),
                [self, this, message](const boost::system::error_code &ec, std::size_t /*length*/) -> void {
                if (!ec) {
                    access_event_handler().receive_message(message);
                } else {
                    close();
                }
            });
        } else {
            close();
        }
    });
}

void BasicSession::close() {
    if (access_socket().is_open()) {
        access_socket().close();
        access_event_handler().close_session();
    }
}

void BasicSession::_send_next() {
    if (_message_queue.empty()) return;

    std::string message = std::move(_message_queue.front());

    auto self = shared_from_this();

    size_t message_length = hton64(message.size());
    boost::asio::async_write(access_socket(), boost::asio::buffer(&message_length, sizeof(message_length)),
        [self, this, message](const boost::system::error_code &ec, std::size_t /*length*/) -> void {
        if (!ec) {
            boost::asio::async_write(access_socket(), boost::asio::buffer(message),
                [self, this, message](const boost::system::error_code &ec, std::size_t /*length*/) -> void {
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
