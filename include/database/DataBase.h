#pragma once

#include <sqlite3.h>
#include <stdexcept>
#include <string>
#include "Statement.h"

namespace oink_judge::database {

class DataBase {
public:

    DataBase(const DataBase &config) = delete;
    DataBase &operator=(const DataBase &) = delete;

    static DataBase &instance();

    template <typename... Args>
    void prepare_statement(Statement &statement, const std::string &sql_template, const Args&... args) const;
    void prepare_statement(Statement &statement, const std::string &sql_template) const;

private:
    DataBase();

    sqlite3 *_db;
};

namespace {
    int now_argument_place;

    void bind_parameters(Statement &stmt, const std::string &arg) {
        stmt.bind_text(arg, now_argument_place++);
    }

    void bind_parameters(Statement &stmt, int arg) {
        stmt.bind_int(arg, now_argument_place++);
    }

    template <typename... Args>
    void bind_parameters(Statement &stmt, const std::string &arg1, const Args&... args) {
        stmt.bind_text(arg1, now_argument_place++);
        bind_parameters(stmt, args...);
    }

    template <typename... Args>
    void bind_parameters(Statement &stmt, int arg1, const Args&... args) {
        stmt.bind_int(arg1, now_argument_place++);
        bind_parameters(stmt, args...);
    }
} // namespace


template <typename... Args>
void DataBase::prepare_statement(Statement &statement, const std::string &sql_template, const Args&... args) const {
    int rc = sqlite3_prepare_v2(_db, sql_template.c_str(), -1, statement.get_stmt(), nullptr);
    if (rc != SQLITE_OK) {
        throw std::runtime_error("Failed to prepare statement: " + std::string(sqlite3_errmsg(_db)));
    }

    now_argument_place = 1;
    bind_parameters(statement, args...);
}

} // namespace oink_judge::database
