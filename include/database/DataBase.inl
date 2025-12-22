#pragma once

#include "DataBase.h"

namespace oink_judge::database {

template <typename... Args>
pqxx::result DataBase::execute(const std::string &sql_template_name, const Args&... args) {
    pqxx::work txn(_conn);

    pqxx::params params;
    (params.append(args), ...);

    pqxx::result res = txn.exec(pqxx::prepped(sql_template_name), params);
    txn.commit();

    return res;
}

template <typename... Args>
pqxx::result DataBase::execute_sql(const std::string &sql, const Args&... args) {
    pqxx::work txn(_conn);

    pqxx::params params;
    (params.append(args), ...);

    pqxx::result res = txn.exec(sql, params);
    txn.commit();

    return res;
}

template <typename... Args>
pqxx::result DataBase::execute_read_only(const std::string &sql_template_name, const Args&... args) {
    pqxx::nontransaction txn(_conn);
    
    pqxx::params params;
    (params.append(args), ...);

    pqxx::result res = txn.exec(pqxx::prepped(sql_template_name), params);

    return res;
}

template <typename... Args>
pqxx::result DataBase::execute_sql_read_only(const std::string &sql, const Args&... args) {
    pqxx::nontransaction txn(_conn);

    pqxx::params params;
    (params.append(args), ...);

    pqxx::result res = txn.exec(sql, params);

    return res;
}

} // namespace oink_judge::database