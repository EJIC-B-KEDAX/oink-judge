#include "database/DataBase.h"
#include "config/Config.h"

namespace oink_judge::database {

using Config = config::Config;

DataBase &DataBase::instance() {
    static DataBase instance;
    return instance;
}

std::string DataBase::quote(const std::string &value) const {
    return _conn.quote(value);
}

std::string DataBase::quote_name(const std::string &name) const {
    return _conn.quote_name(name);
}

std::string DataBase::quote_columns(const std::vector<std::string> &columns) const {
    return _conn.quote_columns(columns);
}

std::string DataBase::quote_table(const std::string &table) const {
    return _conn.quote_table(table);
}


void DataBase::prepare_statement(const std::string &sql_template_name, const std::string &sql_template) {
    _prepared_statements.insert(sql_template_name);
    _conn.prepare(sql_template_name, sql_template);
}

void DataBase::unprepare_statement(const std::string &sql_template_name) {
    _prepared_statements.erase(sql_template_name);
    _conn.unprepare(sql_template_name);
}


pqxx::result DataBase::execute(const std::string &sql_template_name) {
    pqxx::work txn(_conn);
    pqxx::result res = txn.exec_prepared(sql_template_name);
    txn.commit();

    return res;
}

pqxx::result DataBase::execute_sql(const std::string &sql) {
    pqxx::work txn(_conn);
    pqxx::result res = txn.exec(sql);
    txn.commit();

    return res;
}

pqxx::result DataBase::execute_read_only(const std::string &sql_template_name) {
    pqxx::nontransaction txn(_conn);
    pqxx::result res = txn.exec_prepared(sql_template_name);

    return res;
}

pqxx::result DataBase::execute_sql_read_only(const std::string &sql) {
    pqxx::nontransaction txn(_conn);
    pqxx::result res = txn.exec(sql);

    return res;
}

bool DataBase::is_statement_prepared(const std::string &sql_template_name) const {
    return _prepared_statements.contains(sql_template_name);
}

DataBase::DataBase() : _conn(
    "host=" + static_cast<std::string>(Config::config()["database"]["host"]) +
    " port=" + static_cast<std::string>(Config::config()["ports"]["database"]) +
    " dbname=" + static_cast<std::string>(Config::config()["database"]["name"]) +
    " user=" + static_cast<std::string>(Config::config()["database"]["username"]) +
    " password=" + static_cast<std::string>(Config::credentials()["database"]["password"])) {}

} // namespace oink_judge::database
