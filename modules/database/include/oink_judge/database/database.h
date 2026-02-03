#pragma once
#include <pqxx/pqxx>
#include <string>
#include <unordered_set>

namespace oink_judge::database {

class DataBase {
  public:
    DataBase(const DataBase& config) = delete;
    auto operator=(const DataBase&) -> DataBase& = delete;
    DataBase(DataBase&&) = delete;
    auto operator=(DataBase&&) -> DataBase& = delete;
    ~DataBase();

    static auto instance() -> DataBase&;

    auto quote(const std::string& value) const -> std::string;
    auto quoteName(const std::string& name) const -> std::string;
    auto quoteTable(const std::string& table) const -> std::string;
    auto quoteColumns(const std::vector<std::string>& columns) const -> std::string;

    auto prepareStatement(const std::string& sql_template_name, const std::string& sql_template) -> void;
    auto unprepareStatement(const std::string& sql_template_name) -> void;

    template <typename... Args> auto execute(const std::string& sql_template_name, const Args&... args) -> pqxx::result;
    auto execute(const std::string& sql_template_name) -> pqxx::result;
    template <typename... Args> auto executeSQL(const std::string& sql, const Args&... args) -> pqxx::result;
    auto executeSQL(const std::string& sql) -> pqxx::result;
    template <typename... Args>
    [[nodiscard]] auto executeReadOnly(const std::string& sql_template_name, const Args&... args) const -> pqxx::result;
    [[nodiscard]] auto executeReadOnly(const std::string& sql_template_name) const -> pqxx::result;
    template <typename... Args>
    [[nodiscard]] auto executeSQLReadOnly(const std::string& sql, const Args&... args) const -> pqxx::result;
    [[nodiscard]] auto executeSQLReadOnly(const std::string& sql) const -> pqxx::result;

    [[nodiscard]] auto isStatementPrepared(const std::string& sql_template_name) const -> bool;

  private:
    DataBase();

    std::string connection_string_;
    mutable pqxx::connection connection_;

    std::unordered_set<std::string> prepared_statements_;

    void ensureConnection() const;
};

} // namespace oink_judge::database

#include "database.inl"
