#pragma once
#include "database.h"

namespace oink_judge::database {

template <typename... Args> pqxx::result DataBase::execute(const std::string& sql_template_name, const Args&... args) {
    ensureConnection();

    pqxx::work txn(connection_);
    pqxx::params params;
    (params.append(args), ...);

    pqxx::result res = txn.exec(pqxx::prepped(sql_template_name), params);
    txn.commit();

    return res;
}

template <typename... Args> auto DataBase::executeSQL(const std::string& sql, const Args&... args) -> pqxx::result {
    ensureConnection();

    pqxx::work txn(connection_);
    pqxx::params params;
    (params.append(args), ...);

    pqxx::result res = txn.exec(sql, params);
    txn.commit();

    return res;
}

template <typename... Args>
auto DataBase::executeReadOnly(const std::string& sql_template_name, const Args&... args) const -> pqxx::result {
    ensureConnection();

    pqxx::nontransaction txn(connection_);
    pqxx::params params;
    (params.append(args), ...);

    pqxx::result res = txn.exec(pqxx::prepped(sql_template_name), params);

    return res;
}

template <typename... Args> auto DataBase::executeSQLReadOnly(const std::string& sql, const Args&... args) const -> pqxx::result {
    ensureConnection();

    pqxx::nontransaction txn(connection_);
    pqxx::params params;
    (params.append(args), ...);

    pqxx::result res = txn.exec(sql, params);

    return res;
}

} // namespace oink_judge::database
