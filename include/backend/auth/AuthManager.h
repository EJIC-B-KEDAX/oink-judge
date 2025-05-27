#ifndef OINK_JUDGE_BACKEND_AUTH_AUTH_MANAGER_H
#define OINK_JUDGE_BACKEND_AUTH_AUTH_MANAGER_H

#include "backend/auth/AuthDB.h"
#include "backend/auth/Session.h"
#include <unordered_map>

namespace oink_judge::backend::auth {

class AuthManager {
public:
    AuthManager(const char *db_path);
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
    AuthDB _auth_db;
    std::unordered_map<std::string, Session> _sessions;
};

} // namespace oink_judge::backend::auth

#endif //OINK_JUDGE_BACKEND_AUTH_AUTH_MANAGER_H
