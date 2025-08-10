#include "socket/ConnectionStorage.h"

namespace oink_judge::socket {

ConnectionStorage& ConnectionStorage::instance() {
    static ConnectionStorage instance;
    return instance;
}

void ConnectionStorage::insert_connection(const std::shared_ptr<Session> &session) {
    _connections.push_back(session);
}

void ConnectionStorage::remove_connection(const std::shared_ptr<Session> &session) {
    auto it = std::ranges::find(_connections, session);
    if (it != _connections.end()) {
        _connections.erase(it, _connections.end());
    }
}

ConnectionStorage::ConnectionStorage() = default;

} // namespace oink_judge::socket
