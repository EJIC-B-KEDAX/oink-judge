#pragma once
#include <libpq-fe.h>

#include <cstddef>
#include <string>

namespace oink_judge::database {

class QueryField {
  public:
    QueryField(PGresult* result, int row, int column);

    [[nodiscard]] auto isNull() const -> bool;
    template <typename T> [[nodiscard]] auto as() const -> T;

  private:
    PGresult* result_;
    int row_;
    int column_;
};

class QueryRow {
  public:
    QueryRow(PGresult* result, int row);

    [[nodiscard]] auto size() const -> int;
    [[nodiscard]] auto field(int column) const -> QueryField;
    [[nodiscard]] auto operator[](int column) const -> QueryField;
    [[nodiscard]] auto operator[](const std::string& column_name) const -> QueryField;

  private:
    PGresult* result_;
    int row_;
};

class QueryResult {
  public:
    QueryResult();
    explicit QueryResult(PGresult* result);

    QueryResult(const QueryResult&) = delete;
    auto operator=(const QueryResult&) -> QueryResult& = delete;
    QueryResult(QueryResult&&) noexcept;
    auto operator=(QueryResult&&) noexcept -> QueryResult&;
    ~QueryResult();

    [[nodiscard]] auto empty() const -> bool;
    [[nodiscard]] auto size() const -> std::size_t;
    [[nodiscard]] auto row(int index) const -> QueryRow;
    [[nodiscard]] auto operator[](int index) const -> QueryRow;

  private:
    PGresult* result_;
};

} // namespace oink_judge::database

#include "oink_judge/database/query_result.inl"
