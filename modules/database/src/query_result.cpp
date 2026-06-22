#include "oink_judge/database/query_result.h"

namespace oink_judge::database {

QueryField::QueryField(PGresult* result, int row, int column) : result_(result), row_(row), column_(column) {}

auto QueryField::isNull() const -> bool { return PQgetisnull(result_, row_, column_) == 1; }

QueryRow::QueryRow(PGresult* result, int row) : result_(result), row_(row) {}

auto QueryRow::size() const -> int { return PQnfields(result_); }

auto QueryRow::field(int column) const -> QueryField { return {result_, row_, column}; }

auto QueryRow::operator[](int column) const -> QueryField { return field(column); }

auto QueryRow::operator[](const std::string& column_name) const -> QueryField {
    const int column = PQfnumber(result_, column_name.c_str());
    if (column < 0) {
        throw std::runtime_error("column not found: " + column_name);
    }
    return field(column);
}

QueryResult::QueryResult() : result_(nullptr) {}

QueryResult::QueryResult(PGresult* result) : result_(result) {}

QueryResult::QueryResult(QueryResult&& other) noexcept : result_(other.result_) { other.result_ = nullptr; }

auto QueryResult::operator=(QueryResult&& other) noexcept -> QueryResult& {
    if (this != &other) {
        if (result_ != nullptr) {
            PQclear(result_);
        }
        result_ = other.result_;
        other.result_ = nullptr;
    }
    return *this;
}

QueryResult::~QueryResult() {
    if (result_ != nullptr) {
        PQclear(result_);
    }
}

auto QueryResult::empty() const -> bool { return result_ == nullptr || PQntuples(result_) == 0; }

auto QueryResult::size() const -> std::size_t {
    if (result_ == nullptr) {
        return 0;
    }
    return static_cast<std::size_t>(PQntuples(result_));
}

auto QueryResult::row(int index) const -> QueryRow { return {result_, index}; }

auto QueryResult::operator[](int index) const -> QueryRow { return row(index); }

} // namespace oink_judge::database
