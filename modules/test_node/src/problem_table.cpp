#include "oink_judge/test_node/problem_table.h"

#include "oink_judge/config/problem_config_utils.h"

#include <cassert>
#include <oink_judge/database/database.h>

namespace oink_judge::test_node {

using DataBase = database::DataBase;

namespace {

auto generateCreateSQL(const std::string& table_name, const std::vector<std::string>& colums) -> std::string {
    std::string create_sql = "CREATE TABLE IF NOT EXISTS " + table_name + " (\n";
    bool need_comma = false;
    for (const auto& column : colums) {
        if (need_comma) {
            create_sql += ",\n";
        }
        create_sql += column + " ";
        if (column == DataBase::instance().quoteName("username")) {
            create_sql += "TEXT PRIMARY KEY";
        } else if (column == DataBase::instance().quoteName("tested_on_revision")) {
            create_sql += "INTEGER DEFAULT 0";
        } else {
            create_sql += "REAL DEFAULT 0";
        }
        need_comma = true;
    }
    create_sql += ");";

    return create_sql;
}

} // namespace

ProblemTable::ProblemTable(std::string problem_id) : id_(std::move(problem_id)) {
    touchTable();

    checkTableActuality();

    const std::string SET_TOTAL_SCORE_SQL = "UPDATE " + getTableName() + " SET total_score = $2 WHERE username = $1";

    const std::string SET_TESTED_ON_REVISION_SQL =
        "UPDATE " + getTableName() + " SET tested_on_revision = $2 WHERE username = $1";

    const std::string ADD_NEW_USER_SQL = "INSERT INTO " + getTableName() + " (username) VALUES ($1)";

    const std::string SELECT_USER_LINE_SQL = "SELECT 1 FROM " + getTableName() + " WHERE username = $1";

    const std::string GET_TOTAL_SCORE_SQL = "SELECT total_score FROM " + getTableName() + " WHERE username = $1";

    const std::string GET_TESTED_ON_REVISION_SQL = "SELECT tested_on_revision FROM " + getTableName() + " WHERE username = $1";

    DataBase::instance().prepareStatement(getTableName() + "__set_total_score", SET_TOTAL_SCORE_SQL);
    DataBase::instance().prepareStatement(getTableName() + "__set_tested_on_revision", SET_TESTED_ON_REVISION_SQL);
    DataBase::instance().prepareStatement(getTableName() + "__add_new_user", ADD_NEW_USER_SQL);
    DataBase::instance().prepareStatement(getTableName() + "__select_user_line", SELECT_USER_LINE_SQL);
    DataBase::instance().prepareStatement(getTableName() + "__get_total_score", GET_TOTAL_SCORE_SQL);
    DataBase::instance().prepareStatement(getTableName() + "__get_tested_on_revision", GET_TESTED_ON_REVISION_SQL);

    // TODO add statement for set full string
}

auto ProblemTable::checkTableActuality() -> bool {
    std::vector<std::string> columns = getColumns();
    std::vector<std::string> right_columns = getRightColumns();

    if (columns == right_columns) {
        return true;
    }

    {
        assert(columns.size() >= 3);

        for (int i = 0; i < 3; i++) {
            assert(columns[i] == right_columns[i]);
        }
    }

    for (int i = 3; i < columns.size(); i++) {
        std::string drop_sql = "ALTER TABLE " + getTableName() + " DROP COLUMN " + columns[i] + ";";

        DataBase::instance().executeSQL(drop_sql);
    }

    for (int i = 3; i < right_columns.size(); i++) {
        std::string add_sql = "ALTER TABLE " + getTableName() + " ADD COLUMN " + right_columns[i] + " REAL DEFAULT 0;";
        DataBase::instance().executeSQL(add_sql);
    }

    return false;
}

auto ProblemTable::getTableName() const -> std::string { return DataBase::instance().quoteTable("problem_" + id_); }

auto ProblemTable::getRawTableName() const -> std::string { return "problem_" + id_; }

auto ProblemTable::getColumns() const -> std::vector<std::string> {
    std::string sql = "SELECT column_name "
                      "FROM information_schema.columns "
                      "WHERE table_name = $1 "
                      "ORDER BY ordinal_position;";

    pqxx::result res = DataBase::instance().executeSQLReadOnly(sql, getRawTableName());

    std::vector<std::string> columns;

    for (auto row : res) {
        columns.push_back(DataBase::instance().quoteName(row["column_name"].as<std::string>()));
    }

    return columns;
}

auto ProblemTable::setTotalScore(const std::string& username, double total_score) -> void {
    addNewUser(username);

    DataBase::instance().execute(getTableName() + "__set_total_score", username, total_score);
}

auto ProblemTable::setTestedOnRevision(const std::string& username, int revision) -> void {
    addNewUser(username);

    DataBase::instance().execute(getTableName() + "__set_tested_on_revision", username, revision);
}

auto ProblemTable::setScoreOnTest(const std::string& username, const std::shared_ptr<Test>& test, double score) -> void {
    addNewUser(username);

    std::string column_name = DataBase::instance().quoteName("score_on_" + test->getName());

    std::string sql = "UPDATE " + getTableName() + " SET " + column_name + " = $2 WHERE username = $1";

    DataBase::instance().executeSQL(sql, username, score);
}

auto ProblemTable::addNewUser(const std::string& username) -> void { // NOLINT(readability-make-member-function-const)
    if (doesUserHaveSubmission(username)) {
        return;
    }

    DataBase::instance().execute(getTableName() + "__add_new_user", username);
}

auto ProblemTable::getTotalScore(const std::string& username) const -> double {
    pqxx::result res = DataBase::instance().executeReadOnly(getTableName() + "__get_total_score", username);

    if (res.empty()) {
        return 0.0;
    }

    return res[0]["total_score"].as<double>();
}

auto ProblemTable::getTestedOnRevision(const std::string& username) const -> int {
    pqxx::result res = DataBase::instance().executeReadOnly(getTableName() + "__get_tested_on_revision", username);

    if (res.empty()) {
        return 0;
    }

    return res[0]["total_score"].as<int>();
}

auto ProblemTable::getScoreOnTest(const std::string& username, const std::shared_ptr<Test>& test) const -> double {
    std::string column_name = DataBase::instance().quoteName("score_on_" + test->getName());

    std::string sql = "SELECT " + column_name + " FROM " + getTableName() + " WHERE username = $1";

    pqxx::result res = DataBase::instance().executeSQLReadOnly(sql, username);

    if (res.empty()) {
        return 0.0;
    }

    return res[0][column_name].as<double>();
}

auto ProblemTable::doesUserHaveSubmission(const std::string& username) const -> bool {
    pqxx::result res = DataBase::instance().executeReadOnly(getTableName() + "__select_user_line", username);

    return !res.empty();
}

auto ProblemTable::getRightColumns() const -> std::vector<std::string> {
    std::vector<std::string> columns = {DataBase::instance().quoteName("username"), DataBase::instance().quoteName("total_score"),
                                        DataBase::instance().quoteName("tested_on_revision")};

    std::vector<std::string> test_names = problem_config::getAllTestNames(id_).value_or(std::vector<std::string>{});

    for (const auto& test_name : test_names) {
        columns.push_back(DataBase::instance().quoteName("score_on_" + test_name));
    }

    return columns;
}

auto ProblemTable::touchTable() -> void { DataBase::instance().executeSQL(generateCreateSQL(getTableName(), getRightColumns())); }

} // namespace oink_judge::test_node
