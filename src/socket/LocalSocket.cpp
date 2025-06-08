#include "LocalSocket.h"
#include <stdexcept>
#include <unistd.h>
#include "config/Config.h"

namespace oink_judge::socket {

LocalSocket::LocalSocket(int port, in_addr_t address) {
    MAX_CONNECTIONS = config::Config::instance().get_bound("max_connections");

    _socket_fd = ::socket(AF_INET, SOCK_STREAM, 0);
    if (_socket_fd < 0) {
        throw std::runtime_error("Failed to create socket");
    }

    sockaddr_in server_address{};
    server_address.sin_family = AF_INET;
    server_address.sin_addr.s_addr = address;
    server_address.sin_port = htons(port);

    if (bind(_socket_fd, (sockaddr*)&server_address, sizeof(server_address)) < 0) {
        close(_socket_fd);
        throw std::runtime_error("Bind failed");
    }

    if (listen(_socket_fd, MAX_CONNECTIONS) < 0) {
        close(_socket_fd);
        throw std::runtime_error("Listen failed");
    }
}

LocalSocket::~LocalSocket() {
    if (_socket_fd >= 0) {
        close(_socket_fd);
    }
}

ClientSocket LocalSocket::accept() const {
    int client_socket_fd = ::accept(_socket_fd, nullptr, nullptr);
    if (client_socket_fd < 0) {
        throw std::runtime_error("Accept failed");
    }
    return ClientSocket(client_socket_fd);
}

} // namespace oink_judge::socket