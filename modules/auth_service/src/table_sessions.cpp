#include "oink_judge/auth_service/table_sessions.h"

#include <ctime>
#include <iostream>
#include <oink_judge/database/database.h>

namespace oink_judge::auth_service {

using DataBase = database::DataBase;

auto TableSessions::instance() -> TableSessions& {
    static TableSessions instance;
    return instance;
}

auto TableSessions::addSession(const Session& session) -> bool { // NOLINT
    try {
        DataBase::instance().execute("sessions__add_session", session.getSessionId(), session.getUsername(),
                                     session.getExpireAt());
        return true;
    } catch (const pqxx::sql_error& e) {
        std::cerr << "SQL error: " << e.what() << '\n';
        return false;
    }
}

auto TableSessions::removeSession(const Session& session) -> bool { // NOLINT
    try {
        DataBase::instance().execute("sessions__remove_session", session.getSessionId());
        return true;
    } catch (const pqxx::sql_error& e) {
        std::cerr << "SQL error: " << e.what() << '\n';
        return false;
    }
}

auto TableSessions::removeSession(const std::string& session_id) -> bool { // NOLINT
    try {
        DataBase::instance().execute("sessions__remove_session", session_id);
        return true;
    } catch (const pqxx::sql_error& e) {
        std::cerr << "SQL error: " << e.what() << '\n';
        return false;
    }
}

auto TableSessions::clearAllUserSessions(const std::string& username) -> bool { // NOLINT
    try {
        DataBase::instance().execute("sessions__remove_by_username", username);
        return true;
    } catch (const pqxx::sql_error& e) {
        std::cerr << "SQL error: " << e.what() << '\n';
        return false;
    }
}

auto TableSessions::whoseSession(const std::string& session_id) const -> std::string { // NOLINT
    try {
        pqxx::result res = DataBase::instance().execute("sessions__select_session", session_id);

        if (res.empty()) {
            return "";
        }
        if (res.size() > 1) {
            throw std::runtime_error("Session ID must contain only one session");
        }

        auto stored_username = res[0]["username"].as<std::string>();
        auto stored_expire_at = res[0]["expire_at"].as<time_t>();

        if (isExpired(stored_expire_at)) {
            DataBase::instance().execute("sessions__remove_session", session_id);
            return "";
        }

        return stored_username;
    } catch (const pqxx::sql_error& e) {
        std::cerr << "SQL error: " << e.what() << '\n';
        return "";
    }
}

TableSessions::TableSessions() {
    const std::string CREATE_SQL = "CREATE TABLE IF NOT EXISTS sessions ("
                                   "id TEXT PRIMARY KEY, "
                                   "username TEXT, "
                                   "expire_at INTEGER);";

    DataBase::instance().executeSQL(CREATE_SQL);

    const std::string ADD_SESSION_SQL = "INSERT INTO sessions (id, username, expire_at) VALUES ($1, $2, $3)";
    const std::string REMOVE_SESSION_SQL = "DELETE FROM sessions WHERE id = $1";
    const std::string REMOVE_SESSIONS_BY_USERNAME_SQL = "DELETE FROM sessions WHERE username = $1";
    const std::string SELECT_SESSION_SQL = "SELECT username, expire_at FROM sessions WHERE id = $1";

    DataBase::instance().prepareStatement("sessions__add_session", ADD_SESSION_SQL);
    DataBase::instance().prepareStatement("sessions__remove_session", REMOVE_SESSION_SQL);
    DataBase::instance().prepareStatement("sessions__remove_by_username", REMOVE_SESSIONS_BY_USERNAME_SQL);
    DataBase::instance().prepareStatement("sessions__select_session", SELECT_SESSION_SQL);
}

auto TableSessions::isExpired(time_t expire_at) -> bool { return time(nullptr) > expire_at; }

} // namespace oink_judge::auth_service
