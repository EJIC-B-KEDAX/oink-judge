#include "ClientSocket.h"
#include "LocalSocket.h"
#include <unistd.h>
#include <stdexcept>

namespace oink_judge::socket {

ClientSocket::ClientSocket(int socket_fd) : _socket_fd(socket_fd) {
    if (_socket_fd < 0) {
        throw std::runtime_error("Invalid socket file descriptor");
    }
}

ClientSocket::~ClientSocket() {
    if (_socket_fd >= 0) {
        close(_socket_fd);
    }
}

void ClientSocket::send_int(int value) const {
    int32_t network_value = htonl(value);
    if (send(_socket_fd, &network_value, sizeof(network_value), 0) < 0) {
        throw std::runtime_error("Failed to send integer");
    }
}

int ClientSocket::receive_int() const {
    int32_t network_value;
    if (recv(_socket_fd, &network_value, sizeof(network_value), 0) <= 0) {
        throw std::runtime_error("Failed to receive integer");
    }
    return ntohl(network_value);
}

void ClientSocket::send_string(const std::string &data) const {
    size_t length = data.size();
    send_int(static_cast<int>(length));
    size_t sent = 0;
    while (sent < length) {
        ssize_t bytes_sent = send(_socket_fd, data.c_str() + sent, length - sent, 0);
        if (bytes_sent < 0) {
            throw std::runtime_error("Failed to send string");
        }
        sent += bytes_sent;
    }
}

std::string ClientSocket::receive_string() const {
    int length = receive_int();
    if (length < 0) {
        throw std::runtime_error("Received negative string length");
    }
    
    std::string data;
    data.resize(length);
    size_t received = 0;
    
    while (received < length) {
        ssize_t bytes_received = recv(_socket_fd, &data[received], length - received, 0);
        if (bytes_received <= 0) {
            throw std::runtime_error("Failed to receive string");
        }
        received += bytes_received;
    }
    
    return data;
}

void ClientSocket::send_json(const json &data) const {
    std::string json_str = data.dump();
    send_string(json_str);
}

json ClientSocket::receive_json() const {
    std::string json_str = receive_string();
    try {
        return json::parse(json_str);
    } catch (const json::parse_error &e) {
        throw std::runtime_error("Failed to parse JSON: " + std::string(e.what()));
    }
}


} // namespace oink_judge::socket