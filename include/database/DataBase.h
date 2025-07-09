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

} // namespace oink_judge::database

#include "DataBase.inl"