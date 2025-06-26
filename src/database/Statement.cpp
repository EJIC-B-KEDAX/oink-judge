#include "database/Statement.h"

namespace oink_judge::database {

Statement::Statement() {
    _stmt = nullptr;
}

Statement::~Statement() {
    if (_stmt != nullptr) {
        sqlite3_finalize(_stmt);
    }
}

void Statement::bind_text(const std::string &value, int place) {
    sqlite3_bind_text(_stmt, place, value.c_str(), -1, SQLITE_STATIC);
}

void Statement::bind_int(int value, int place) {
    sqlite3_bind_int(_stmt, place, value);
}

sqlite3_stmt **Statement::get_stmt() {
    return &_stmt;
}

int Statement::step() {
    return sqlite3_step(_stmt);
}

std::string Statement::column_text(int column) const {
    return reinterpret_cast<const char *>(sqlite3_column_text(_stmt, column));
}

} // namespace oink_judge::database
