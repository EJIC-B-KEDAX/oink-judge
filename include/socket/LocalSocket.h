#pragma once

#include <sys/socket.h>
#include <netinet/in.h>
#include "ClientSocket.h"

namespace oink_judge::socket {


class LocalSocket {
public:
    LocalSocket(int port, in_addr_t address = INADDR_ANY);
    
    LocalSocket(const LocalSocket&) = delete;
    LocalSocket& operator=(const LocalSocket&) = delete;
    
    LocalSocket(LocalSocket&&) = delete;
    LocalSocket& operator=(LocalSocket&&) = delete;

    virtual ~LocalSocket();

    ClientSocket accept() const;

    void close();
private:
    int _socket_fd;

    int MAX_CONNECTIONS;
};

} // namespace oink_judge::socket
