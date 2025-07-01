#include "database/DataBase.h"
#include "config/Config.h"

namespace oink_judge::database {



using Config = config::Config;

DataBase &DataBase::instance() {
    static DataBase instance;
    return instance;
}

void DataBase::prepare_statement(Statement &statement, const std::string &sql_template) const {
    int rc = sqlite3_prepare_v2(_db, sql_template.c_str(), -1, statement.get_stmt(), nullptr);
    if (rc != SQLITE_OK) {
        throw std::runtime_error("Failed to prepare statement: " + std::string(sqlite3_errmsg(_db)));
    }
}

DataBase::DataBase() {
    int rc = sqlite3_open((Config::instance().get_directory("db") + "/data.db").c_str(), &_db); // TODO make this with func get_path_to()
    if (rc != SQLITE_OK) {
        throw std::runtime_error("Failed to open database: " + std::string(sqlite3_errmsg(_db)));
    }
}

} // namespace oink_judge::database
