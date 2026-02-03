#include "oink_judge/database/database.h"

#include <oink_judge/config/config.h>

namespace oink_judge::database {

using Config = config::Config;

DataBase::~DataBase() = default;

auto DataBase::instance() -> DataBase& {
    static DataBase instance;
    return instance;
}

auto DataBase::quote(const std::string& value) const -> std::string {
    ensureConnection();
    return connection_.quote(value);
}

auto DataBase::quoteName(const std::string& name) const -> std::string {
    ensureConnection();
    return connection_.quote_name(name);
}

auto DataBase::quoteColumns(const std::vector<std::string>& columns) const -> std::string {
    ensureConnection();
    return connection_.quote_columns(columns);
}

auto DataBase::quoteTable(const std::string& table) const -> std::string {
    ensureConnection();
    return connection_.quote_table(table);
}

auto DataBase::prepareStatement(const std::string& sql_template_name, const std::string& sql_template) -> void {
    ensureConnection();

    prepared_statements_.insert(sql_template_name);
    connection_.prepare(sql_template_name, sql_template);
}

auto DataBase::unprepareStatement(const std::string& sql_template_name) -> void {
    ensureConnection();
    prepared_statements_.erase(sql_template_name);
    connection_.unprepare(sql_template_name);
}

auto DataBase::execute(const std::string& sql_template_name) -> pqxx::result {
    ensureConnection();

    pqxx::work txn(connection_);
    pqxx::result res = txn.exec(pqxx::prepped(sql_template_name));
    txn.commit();

    return res;
}

auto DataBase::executeSQL(const std::string& sql) -> pqxx::result {
    ensureConnection();

    pqxx::work txn(connection_);
    pqxx::result res = txn.exec(sql);
    txn.commit();

    return res;
}

auto DataBase::executeReadOnly(const std::string& sql_template_name) const -> pqxx::result {
    ensureConnection();

    pqxx::nontransaction txn(connection_);
    pqxx::result res = txn.exec(pqxx::prepped(sql_template_name));

    return res;
}

auto DataBase::executeSQLReadOnly(const std::string& sql) const -> pqxx::result {
    ensureConnection();

    pqxx::nontransaction txn(connection_);
    pqxx::result res = txn.exec(sql);

    return res;
}

auto DataBase::isStatementPrepared(const std::string& sql_template_name) const -> bool {
    return prepared_statements_.contains(sql_template_name);
}

DataBase::DataBase()
    // TODO: use getDatabaseConfig function
    : connection_string_("host=" + static_cast<std::string>(Config::config()["database"]["host"]) +
                         " port=" + std::to_string(static_cast<short>(Config::config()["ports"]["database"])) +
                         " dbname=" + static_cast<std::string>(Config::config()["database"]["dbname"]) +
                         " user=" + static_cast<std::string>(Config::config()["database"]["username"]) +
                         " password=" + static_cast<std::string>(Config::credentials()["database"]["password"]) +
                         " keepalives=1 keepalives_idle=30 keepalives_interval=10 keepalives_count=5"),
      connection_(connection_string_) {}

void DataBase::ensureConnection() const {
    if (!connection_.is_open()) {
        connection_ = pqxx::connection(connection_string_);
    }
}

} // namespace oink_judge::database
