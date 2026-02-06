#pragma once
#include <oink_judge/auth_service/session.h>
#include <string>

namespace oink_judge::auth_service {

class TableSessions {
  public:
    static auto instance() -> TableSessions&;

    TableSessions(const TableSessions&) = delete;
    auto operator=(const TableSessions&) -> TableSessions& = delete;
    TableSessions(TableSessions&&) = delete;
    auto operator=(TableSessions&&) -> TableSessions& = delete;
    ~TableSessions() = default;

    auto addSession(const Session& session) -> bool;

    auto removeSession(const Session& session) -> bool;
    auto removeSession(const std::string& session_id) -> bool;

    auto clearAllUserSessions(const std::string& username) -> bool;

    auto whoseSession(const std::string& session_id) const -> std::string;

  private:
    TableSessions();
    static auto isExpired(time_t expire_at) -> bool;
};

} // namespace oink_judge::auth_service
