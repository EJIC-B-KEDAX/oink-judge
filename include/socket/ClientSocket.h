#pragma once

#include <sys/socket.h>
#include <netinet/in.h>
#include <nlohmann/json.hpp>

using json = nlohmann::json;

namespace oink_judge::socket {

class LocalSocket;

class ClientSocket {
public:
    virtual ~ClientSocket();

    void send_int(int value) const;
    int receive_int() const;

    void send_string(const std::string &data) const;
    std::string receive_string() const;

    void send_json(const json &data) const;
    json receive_json() const;

private:
    ClientSocket(int socket_fd);

    int _socket_fd;

    friend class LocalSocket; // Allow LocalSocket to create ClientSocket instances
};
    
} // namespace oink_judge::socket
