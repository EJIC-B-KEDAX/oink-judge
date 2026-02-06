#include "oink_judge/socket/connection_storage.h"

namespace oink_judge::socket {

auto ConnectionStorage::instance() -> ConnectionStorage& {
    static ConnectionStorage instance;
    return instance;
}

auto ConnectionStorage::insertConnection(const std::shared_ptr<Session>& session) -> void { connections_.push_back(session); }

auto ConnectionStorage::removeConnection(const std::shared_ptr<Session>& session) -> void {
    auto it = std::ranges::find(connections_, session);
    if (it != connections_.end()) {
        connections_.erase(it, connections_.end());
    }
}

ConnectionStorage::ConnectionStorage() = default;

} // namespace oink_judge::socket
