#include "services/auth/AuthManager.h"
#include "config/Config.h"
#include "services/auth/TableUsers.h"
#include "services/auth/TableSessions.h"

namespace oink_judge::services::auth {

AuthManager &AuthManager::instance() {
    static AuthManager instance;
    return instance;
}

AuthManager::~AuthManager() = default;

bool AuthManager::register_user(const std::string &username, const std::string &password) {
    return TableUsers::instance().register_user(username.c_str(), password.c_str());
}

bool AuthManager::user_exists(const std::string &username) {
    return TableUsers::instance().user_exists(username.c_str());
}

bool AuthManager::delete_user(const std::string &username) {
    return TableUsers::instance().delete_user(username.c_str());
}

bool AuthManager::update_password(const std::string &username, const std::string &new_password) {
    return TableUsers::instance().update_password(username.c_str(), new_password.c_str());
}

std::string AuthManager::authenticate(const std::string &username, const std::string &password) {
    if (TableUsers::instance().authenticate(username, password)) {
        Session session(username);
        session.generate_session();
        TableSessions::instance().add_session(session);
        return session.get_session_id();
    }
    return "";
}

std::string AuthManager::whose_session(const std::string &session_id) {
    return TableSessions::instance().whose_session(session_id);
}

bool AuthManager::is_session_valid(const std::string &session_id) {
    return !TableSessions::instance().whose_session(session_id).empty();
}

void AuthManager::invalidate_session(const std::string &session_id) {
    TableSessions::instance().remove_session(session_id);
}

} // namespace oink_judge::services::auth
