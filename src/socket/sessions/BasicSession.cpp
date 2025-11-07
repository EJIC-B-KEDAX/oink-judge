#include "socket/sessions/BasicSession.h"
#include "socket/byte_order.h"
#include <iostream>

namespace oink_judge::socket {

namespace {

[[maybe_unused]] bool registered = []() -> bool {
    BasicSessionFactory::instance().register_type(BasicSession::REGISTERED_NAME, [](const std::string &params, tcp::socket socket) -> std::shared_ptr<Session> {
        auto event_handler = BasicSessionEventHandlerFactory::instance().create(params);

        auto ptr = std::make_shared<BasicSession>(std::move(socket), std::move(event_handler));
        ptr->set_session_ptr();

        return ptr;
    });

    return true;
}();

} // namespace

BasicSession::BasicSession(tcp::socket socket, std::unique_ptr<SessionEventHandler> event_handler) :
    SessionBase(std::move(socket), std::move(event_handler)) {}

void BasicSession::set_session_ptr() {
    access_event_handler().set_session(shared_from_this());
}

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

    auto message_length_net_ptr = std::make_shared<std::array<char, sizeof(size_t)>>();
    boost::asio::async_read(access_socket(), boost::asio::buffer(*message_length_net_ptr),
        [self, this, message_length_net_ptr](const boost::system::error_code &ec, std::size_t /*length*/) -> void {
        if (!ec) {
            size_t message_length_net = 0;
            std::memcpy(&message_length_net, message_length_net_ptr->data(), sizeof(message_length_net));
            size_t message_length = ntoh64(message_length_net);
            auto message_ptr = std::make_shared<std::string>(message_length, '\0');

            boost::asio::async_read(access_socket(), boost::asio::buffer(*message_ptr),
                [self, this, message_ptr](const boost::system::error_code &ec, std::size_t /*length*/) -> void {
                if (!ec) {
                    std::cout << "Received message: " << *message_ptr << std::endl;
                    access_event_handler().receive_message(*message_ptr);
                } else {
                    std::cout << "Error receiving message: " << ec.message() << std::endl;
                    close();
                }
            });
        } else {
            std::cout << "Error receiving message length: " << ec.message() << std::endl;
            close();
        }
    });
}

void BasicSession::close() {
    if (access_socket().is_open()) {
        std::cout << "Closing socket." << std::endl;
        access_socket().close();
        access_event_handler().close_session();
    }
}

void BasicSession::_send_next() {
    if (_message_queue.empty()) return;

    std::string message = std::move(_message_queue.front());
    auto message_ptr = std::make_shared<std::string>(message);

    auto self = shared_from_this();

    size_t message_length = hton64(message.size());
    auto message_length_net_ptr = std::make_shared<std::array<char, sizeof(message_length)>>();
    std::memcpy(message_length_net_ptr->data(), &message_length, sizeof(message_length));
    boost::asio::async_write(access_socket(), boost::asio::buffer(*message_length_net_ptr),
        [self, this, message_ptr, message_length_net_ptr](const boost::system::error_code &ec, std::size_t /*length*/) -> void {
        if (!ec) {
            boost::asio::async_write(access_socket(), boost::asio::buffer(*message_ptr),
                [self, this, message_ptr](const boost::system::error_code &ec, std::size_t /*length*/) -> void {
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
