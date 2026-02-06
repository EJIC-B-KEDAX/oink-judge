#pragma once
#include "oink_judge/socket/session.hpp"

namespace oink_judge::socket {

class ConnectionStorage {
  public:
    static auto instance() -> ConnectionStorage&;

    ConnectionStorage(const ConnectionStorage&) = delete;
    auto operator=(const ConnectionStorage&) -> ConnectionStorage& = delete;
    ConnectionStorage(ConnectionStorage&&) = delete;
    auto operator=(ConnectionStorage&&) -> ConnectionStorage& = delete;
    ~ConnectionStorage();

    auto insertConnection(const std::shared_ptr<Session>& session) -> void;
    auto removeConnection(const std::shared_ptr<Session>& session) -> void;

  private:
    ConnectionStorage();

    std::vector<std::shared_ptr<Session>> connections_;
};

} // namespace oink_judge::socket
