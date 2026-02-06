#include "oink_judge/auth_service/auth_manager.h"

#include "oink_judge/auth_service/table_sessions.h"
#include "oink_judge/auth_service/table_users.h"

namespace oink_judge::auth_service {

auto AuthManager::instance() -> AuthManager& {
    static AuthManager instance;
    return instance;
}

auto AuthManager::registerUser(const std::string& username, const std::string& password) -> bool { // NOLINT
    return TableUsers::instance().registerUser(username, password);
}

auto AuthManager::userExists(const std::string& username) -> bool { // NOLINT
    return TableUsers::instance().userExists(username);
}

auto AuthManager::deleteUser(const std::string& username) -> bool { // NOLINT
    return TableUsers::instance().deleteUser(username);
}

auto AuthManager::updatePassword(const std::string& username, const std::string& new_password) -> bool { // NOLINT
    return TableUsers::instance().updatePassword(username, new_password);
}

auto AuthManager::authenticate(const std::string& username, const std::string& password) -> std::string { // NOLINT
    if (TableUsers::instance().authenticate(username, password)) {
        Session session(username);
        session.generateSession();
        TableSessions::instance().addSession(session);
        return session.getSessionId();
    }
    return "";
}

auto AuthManager::whoseSession(const std::string& session_id) -> std::string { // NOLINT
    return TableSessions::instance().whoseSession(session_id);
}

auto AuthManager::isSessionValid(const std::string& session_id) -> bool { // NOLINT
    return !TableSessions::instance().whoseSession(session_id).empty();
}

auto AuthManager::invalidateSession(const std::string& session_id) -> void { // NOLINT
    TableSessions::instance().removeSession(session_id);
}

} // namespace oink_judge::auth_service
