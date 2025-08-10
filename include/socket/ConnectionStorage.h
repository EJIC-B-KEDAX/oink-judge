#pragma once

#include "Session.hpp"

namespace oink_judge::socket {

class ConnectionStorage {
public:
    static ConnectionStorage& instance();

    ConnectionStorage(const ConnectionStorage&) = delete;
    ConnectionStorage& operator=(const ConnectionStorage&) = delete;

    void insert_connection(const std::shared_ptr<Session> &session);
    void remove_connection(const std::shared_ptr<Session> &session);

private:
    ConnectionStorage();

    std::vector<std::shared_ptr<Session>> _connections;
};

} // namespace oink_judge::socket
