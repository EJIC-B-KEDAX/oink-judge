#include "database/DataBase.h"
#include "config/Config.h"

namespace oink_judge::database {

using Config = config::Config;

DataBase &DataBase::instance() {
    static DataBase instance;
    return instance;
}

std::string DataBase::quote(const std::string &value) const {
    ensure_connection();
    return _conn.quote(value);
}

std::string DataBase::quote_name(const std::string &name) const {
    ensure_connection();
    return _conn.quote_name(name);
}

std::string DataBase::quote_columns(const std::vector<std::string> &columns) const {
    ensure_connection();
    return _conn.quote_columns(columns);
}

std::string DataBase::quote_table(const std::string &table) const {
    ensure_connection();
    return _conn.quote_table(table);
}


void DataBase::prepare_statement(const std::string &sql_template_name, const std::string &sql_template) {
    ensure_connection();

    _prepared_statements.insert(sql_template_name);
    _conn.prepare(sql_template_name, sql_template);
}

void DataBase::unprepare_statement(const std::string &sql_template_name) {
    ensure_connection();

    _prepared_statements.erase(sql_template_name);
    _conn.unprepare(sql_template_name);
}


pqxx::result DataBase::execute(const std::string &sql_template_name) {
    ensure_connection();

    pqxx::work txn(_conn);
    pqxx::result res = txn.exec(pqxx::prepped(sql_template_name));
    txn.commit();

    return res;
}

pqxx::result DataBase::execute_sql(const std::string &sql) {
    ensure_connection();

    pqxx::work txn(_conn);
    pqxx::result res = txn.exec(sql);
    txn.commit();

    return res;
}

pqxx::result DataBase::execute_read_only(const std::string &sql_template_name) const {
    ensure_connection();

    pqxx::nontransaction txn(_conn);
    pqxx::result res = txn.exec(pqxx::prepped(sql_template_name));

    return res;
}

pqxx::result DataBase::execute_sql_read_only(const std::string &sql) const {
    ensure_connection();

    pqxx::nontransaction txn(_conn);
    pqxx::result res = txn.exec(sql);

    return res;
}

bool DataBase::is_statement_prepared(const std::string &sql_template_name) const {
    return _prepared_statements.contains(sql_template_name);
}

DataBase::DataBase() : _connection_string(
    "host=" + static_cast<std::string>(Config::config()["database"]["host"]) +
    " port=" + std::to_string(static_cast<short>(Config::config()["ports"]["database"])) +
    " dbname=" + static_cast<std::string>(Config::config()["database"]["dbname"]) +
    " user=" + static_cast<std::string>(Config::config()["database"]["username"]) +
    " password=" + static_cast<std::string>(Config::credentials()["database"]["password"]) + 
    " keepalives=1 keepalives_idle=30 keepalives_interval=10 keepalives_count=5"),
    _conn(_connection_string) {}

void DataBase::ensure_connection() const {
    if (!_conn.is_open()) {
        _conn = pqxx::connection(_connection_string);
    }
}

} // namespace oink_judge::database
