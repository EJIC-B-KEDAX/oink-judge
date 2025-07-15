#pragma once

#include <pqxx/pqxx>
#include <string>
#include <unordered_set>

namespace oink_judge::database {

class DataBase {
public:
    DataBase(const DataBase &config) = delete;
    DataBase &operator=(const DataBase &) = delete;

    static DataBase &instance();

    std::string quote(const std::string &value) const;
    std::string quote_name(const std::string &name) const;
    std::string quote_table(const std::string &table) const;
    std::string quote_columns(const std::vector<std::string> &columns) const;

    void prepare_statement(const std::string &sql_template_name, const std::string &sql_template);
    void unprepare_statement(const std::string &sql_template_name);

    template <typename... Args>
    pqxx::result execute(const std::string &sql_template_name, const Args&... args);
    pqxx::result execute(const std::string &sql_template_name);
    template <typename... Args>
    pqxx::result execute_sql(const std::string &sql, const Args&... args);
    pqxx::result execute_sql(const std::string &sql);
    template <typename... Args>
    pqxx::result execute_read_only(const std::string &sql_template_name, const Args&... args);
    pqxx::result execute_read_only(const std::string &sql_template_name);
    template <typename... Args>
    pqxx::result execute_sql_read_only(const std::string &sql, const Args&... args);
    pqxx::result execute_sql_read_only(const std::string &sql);

    bool is_statement_prepared(const std::string &sql_template_name) const;

private:
    DataBase();

    pqxx::connection _conn;

    std::unordered_set<std::string> _prepared_statements;
};

} // namespace oink_judge::database

#include "DataBase.inl"