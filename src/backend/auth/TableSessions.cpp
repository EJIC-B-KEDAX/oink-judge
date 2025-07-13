#include "backend/auth/TableSessions.h"
#include <ctime>
#include "database/DataBase.h"

namespace oink_judge::backend::auth {

using DataBase = database::DataBase;
using Statement = database::Statement;

TableSessions &TableSessions::instance() {
    static TableSessions instance;
    return instance;
}

bool TableSessions::add_session(const Session &session) {
    Statement statement;
    std::string sql = "INSERT INTO sessions (id, username, expire_at) VALUES (?, ?, ?)";

    DataBase::instance().prepare_statement(statement, sql, session.get_session_id(), session.get_username(), session.get_expire_at());

    return statement.step() == SQLITE_DONE;
}

bool TableSessions::remove_session(const Session &session) {
    Statement statement;
    std::string sql = "DELETE FROM sessions WHERE id = ?";

    DataBase::instance().prepare_statement(statement, sql, session.get_session_id());

    return statement.step() == SQLITE_DONE;
}

bool TableSessions::remove_session(const std::string &session_id) {
    Statement statement;
    std::string sql = "DELETE FROM sessions WHERE id = ?";

    DataBase::instance().prepare_statement(statement, sql, session_id);

    return statement.step() == SQLITE_DONE;
}


bool TableSessions::clear_all_user_sessions(const std::string &username) {
    Statement statement;
    std::string sql = "DELETE FROM sessions WHERE username = ?";

    DataBase::instance().prepare_statement(statement, sql, username);

    return statement.step() == SQLITE_DONE;
}

std::string TableSessions::whose_session(const std::string &session_id) const {
    Statement statement;
    std::string sql = "SELECT username, expire_at FROM sessions WHERE id = ?";

    DataBase::instance().prepare_statement(statement, sql, session_id);

    if (statement.step() == SQLITE_ROW) {
        std::string stored_username = statement.column_text(0);
        time_t expire_at = statement.column_int64(1);
        if (!_is_expired(expire_at)) {
            return stored_username;
        }

        Statement statement2;
        std::string sql2 = "DELETE FROM sessions WHERE id = ?";

        DataBase::instance().prepare_statement(statement2, sql2, session_id);

        statement2.step();
    }

    return "";
}

TableSessions::TableSessions() {
    Statement statement;

    std::string sql = "CREATE TABLE IF NOT EXISTS sessions ("
                      "id TEXT PRIMARY KEY, "
                      "username TEXT, "
                      "expire_at INTEGER);";

    DataBase::instance().prepare_statement(statement, sql);
    statement.step();
}


bool TableSessions::_is_expired(time_t expire_at) const {
    return time(nullptr) > expire_at;
}

} // namespace oink_judge::backend::auth