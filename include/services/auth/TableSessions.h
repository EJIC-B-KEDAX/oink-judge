#pragma once
#include "services/auth/Session.h"

#include <string>

namespace oink_judge::services::auth {

class TableSessions {
  public:
    static TableSessions& instance();

    TableSessions(const TableSessions&) = delete;
    TableSessions& operator=(const TableSessions&) = delete;

    bool add_session(const Session& session);

    bool remove_session(const Session& session);
    bool remove_session(const std::string& session_id);

    bool clear_all_user_sessions(const std::string& username);

    std::string whose_session(const std::string& session_id) const;

  private:
    TableSessions();
    static bool _is_expired(time_t expire_at);
};

} // namespace oink_judge::services::auth
