#include "backend/auth/AuthManager.h"
#include "config/Config.h"
#include "backend/auth/TableUsers.h"

namespace oink_judge::backend::auth {

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
        _sessions[session.get_session_id()] = session;
        return session.get_session_id();
    }
    return "";
}

std::string AuthManager::whose_session(const std::string &session_id) {
    auto it = _sessions.find(session_id);
    if (it != _sessions.end() && it->second.get_session_id() == session_id) {
        std::string username = it->second.get_username();
        if (!user_exists(username)) {
            it->second.invalidate();
            _sessions.erase(it);
        }
        return it->second.get_username();
    }
    if (it != _sessions.end()) {
        it->second.invalidate();
        _sessions.erase(it);
    }
    return "";
}

bool AuthManager::is_session_valid(const std::string &session_id) {
    auto it = _sessions.find(session_id);
    if (it != _sessions.end() && it->second.is_valid()) {
        return true;
    }
    return false;
}

void AuthManager::invalidate_session(const std::string &session_id) {
    auto it = _sessions.find(session_id);
    if (it != _sessions.end()) {
        it->second.invalidate();
        _sessions.erase(it);
    }
}

} // namespace oink_judge::backend::auth