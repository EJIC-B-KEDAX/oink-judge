#include "backend/auth/TableSessions.h"
#include <ctime>
#include <iostream>
#include "database/DataBase.h"

namespace oink_judge::backend::auth {

using DataBase = database::DataBase;

TableSessions &TableSessions::instance() {
    static TableSessions instance;
    return instance;
}

bool TableSessions::add_session(const Session &session) {
    try {
        DataBase::instance().execute("sessions__add_session", session.get_session_id(), session.get_username(), session.get_expire_at());
        return true;
    } catch (const pqxx::sql_error& e) {
        std::cerr << "SQL error: " << e.what() << std::endl;
        return false;
    }
}

bool TableSessions::remove_session(const Session &session) {
    try {
        DataBase::instance().execute("sessions__remove_session", session.get_session_id());
        return true;
    } catch (const pqxx::sql_error& e) {
        std::cerr << "SQL error: " << e.what() << std::endl;
        return false;
    }
}

bool TableSessions::remove_session(const std::string &session_id) {
    try {
        DataBase::instance().execute("sessions__remove_session", session_id);
        return true;
    } catch (const pqxx::sql_error& e) {
        std::cerr << "SQL error: " << e.what() << std::endl;
        return false;
    }
}


bool TableSessions::clear_all_user_sessions(const std::string &username) {
    try {
        DataBase::instance().execute("sessions__remove_by_username", username);
        return true;
    } catch (const pqxx::sql_error& e) {
        std::cerr << "SQL error: " << e.what() << std::endl;
        return false;
    }
}

std::string TableSessions::whose_session(const std::string &session_id) const {
    try {
        pqxx::result res = DataBase::instance().execute("sessions__select_session", session_id);

        if (res.empty()) return "";
        if (res.size() > 1) {
            throw std::runtime_error("Session ID must contain only one session");
        }

        auto stored_username = res[0]["username"].as<std::string>();
        auto stored_expire_at = res[0]["expire_at"].as<time_t>();

        if (_is_expired(stored_expire_at)) {
            DataBase::instance().execute("sessions__remove_session", session_id);
            return "";
        }

        return stored_username;
    } catch (const pqxx::sql_error& e) {
        std::cerr << "SQL error: " << e.what() << std::endl;
        return "";
    }
}

TableSessions::TableSessions() {
    const std::string create_sql = "CREATE TABLE IF NOT EXISTS sessions ("
                                       "id TEXT PRIMARY KEY, "
                                       "username TEXT, "
                                       "expire_at INTEGER);";

    DataBase::instance().execute_sql(create_sql);

    const std::string add_session_sql = "INSERT INTO sessions (id, username, expire_at) VALUES ($1, $2, $3)";

    const std::string remove_session_sql = "DELETE FROM sessions WHERE id = $1";

    const std::string remove_sessions_by_username_sql = "DELETE FROM sessions WHERE username = $1";

    const std::string select_session_sql = "SELECT username, expire_at FROM sessions WHERE id = $1";


    DataBase::instance().prepare_statement("sessions__add_session", add_session_sql);
    DataBase::instance().prepare_statement("sessions__remove_session", remove_session_sql);
    DataBase::instance().prepare_statement("sessions__remove_by_username", remove_sessions_by_username_sql);
    DataBase::instance().prepare_statement("sessions__select_session", select_session_sql);
}


bool TableSessions::_is_expired(time_t expire_at) {
    return time(nullptr) > expire_at;
}

} // namespace oink_judge::backend::auth