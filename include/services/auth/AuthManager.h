#pragma once

#include "services/auth/Session.h"
#include <unordered_map>

namespace oink_judge::services::auth {

class AuthManager {
public:
    static AuthManager &instance();
    ~AuthManager();

    AuthManager(const AuthManager &) = delete;
    AuthManager &operator=(const AuthManager &) = delete;
    AuthManager(AuthManager &&) = delete;
    AuthManager &operator=(AuthManager &&) = delete;

    bool register_user(const std::string &username, const std::string &password);
    bool user_exists(const std::string &username);
    bool delete_user(const std::string &username);
    bool update_password(const std::string &username, const std::string &new_password);

    std::string authenticate(const std::string &username, const std::string &password);
    std::string whose_session(const std::string &session_id);
    bool is_session_valid(const std::string &session_id);
    void invalidate_session(const std::string &session_id);

private:
    AuthManager() = default;
};

} // namespace oink_judge::services::auth
