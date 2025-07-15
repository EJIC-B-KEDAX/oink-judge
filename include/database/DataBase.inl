#pragma once

#include "DataBase.h"

namespace oink_judge::database {

template <typename... Args>
pqxx::result DataBase::execute(const std::string &sql_template_name, const Args&... args) {
    pqxx::work txn(_conn);
    pqxx::result res = txn.exec_prepared(sql_template_name, args...);
    txn.commit();

    return res;
}

template <typename... Args>
pqxx::result DataBase::execute_sql(const std::string &sql, const Args&... args) {
    pqxx::work txn(_conn);
    pqxx::result res = txn.exec_params(sql, args...);
    txn.commit();

    return res;
}

template <typename... Args>
pqxx::result DataBase::execute_read_only(const std::string &sql_template_name, const Args&... args) {
    pqxx::nontransaction txn(_conn);
    pqxx::result res = txn.exec_prepared(sql_template_name, args...);

    return res;
}

template <typename... Args>
pqxx::result DataBase::execute_sql_read_only(const std::string &sql, const Args&... args) {
    pqxx::nontransaction txn(_conn);
    pqxx::result res = txn.exec_params(sql, args...);

    return res;
}

} // namespace oink_judge::database