#include "backend/management/ProblemTable.h"
#include "database/DataBase.h"
#include <cassert>

namespace oink_judge::backend::management {

using DataBase = database::DataBase;
using Statement = database::Statement;

ProblemTable::ProblemTable(const ProblemConfig &problem_config) : _config(problem_config) {
    _id = _config.get_id();
    _short_name = _config.get_short_name();

    touch_table();

    check_table_actuality();
}

bool ProblemTable::check_table_actuality() {
    std::vector<std::string> columns = get_columns();
    std::vector<std::string> right_columns = get_right_columns();

    if (columns == right_columns) {
        return true;
    }

    {
        assert(columns.size() >= 3);
        assert(columns[0] == "username");
        assert(columns[1] == "total_score");
        assert(columns[2] == "tested_on_revision");
    }

    for (int i = 3; i < columns.size(); i++) {
        Statement statement;
        std::string sql = "ALTER TABLE " + get_table_name() + " DROP COLUMN " + columns[i] + ";";

        DataBase::instance().prepare_statement(statement, sql);

        statement.step();
    }

    for (int i = 3; i < right_columns.size(); i++) {
        Statement statement;
        std::string sql = "ALTER TABLE " + get_table_name() + " ADD COLUMN " + right_columns[i] + " REAL DEFAULT 0;";

        DataBase::instance().prepare_statement(statement, sql);

        statement.step();
    }

    return false;
}

std::string ProblemTable::get_table_name() const {
    return "problem_" + _id + "_" + _short_name;
}

std::vector<std::string> ProblemTable::get_columns() const {
    Statement statement;

    std::string sql = "PRAGMA table_info(" + get_table_name() + ");";

    DataBase::instance().prepare_statement(statement, sql);

    std::vector<std::string> columns;

    while (statement.step() == SQLITE_ROW) {
        columns.push_back(statement.column_text(1));
    }

    return columns;
}

void ProblemTable::set_total_score(const std::string &username, double total_score) {
    add_new_user(username);

    Statement statement;

    std::string sql = "UPDATE " + get_table_name() + " SET total_score = " + std::to_string(total_score) + " WHERE username = ?";

    DataBase::instance().prepare_statement(statement, sql, username);

    statement.step();
}

void ProblemTable::set_tested_on_revision(const std::string &username, int revision) {
    add_new_user(username);

    Statement statement;

    std::string sql = "UPDATE " + get_table_name() + " SET tested_on_revision = " + std::to_string(revision) + " WHERE username = ?";

    DataBase::instance().prepare_statement(statement, sql, username);

    statement.step();
}

void ProblemTable::set_score_on_testset(const std::string &username, const Testset &testset, double score) {    add_new_user(username);
    add_new_user(username);

    Statement statement;

    std::string sql = "UPDATE " + get_table_name() + " SET score_on_" + testset.get_testset_id() + " = " + std::to_string(score) + " WHERE username = ?";

    DataBase::instance().prepare_statement(statement, sql, username);

    statement.step();
}

void ProblemTable::set_score_on_subtask(const std::string &username, const Testset &testset, const Subtask &subtask, double score) {
    add_new_user(username);

    Statement statement;

    std::string sql = "UPDATE " + get_table_name() + " SET score_on_" + testset.get_testset_id() + "_" + subtask.get_subtask_id() + " = " + std::to_string(score) + " WHERE username = ?";

    DataBase::instance().prepare_statement(statement, sql, username);

    statement.step();
}

void ProblemTable::set_score_on_test(const std::string &username, const Testset &testset, const Subtask &subtask, const Test &test, double score) {
    add_new_user(username);

    Statement statement;

    std::string sql = "UPDATE " + get_table_name() + " SET score_on_" + testset.get_testset_id() + "_" + subtask.get_subtask_id() + "_" + test.get_test_id() + " = " + std::to_string(score) + " WHERE username = ?";

    DataBase::instance().prepare_statement(statement, sql, username);

    statement.step();
}

void ProblemTable::add_new_user(const std::string &username) {
    if (does_user_have_submission(username)) return;

    Statement statement;

    std::string sql = "INSERT INTO " + get_table_name() + " (username) VALUES (?)";

    DataBase::instance().prepare_statement(statement, sql, username);

    statement.step();
}

double ProblemTable::get_total_score(const std::string &username) const {
    Statement statement;

    std::string sql = "SELECT total_score FROM " + get_table_name() + " WHERE username = ?";

    DataBase::instance().prepare_statement(statement, sql, username);

    if (statement.step() == SQLITE_ROW) {
        return std::stod(statement.column_text(0));
    }

    return 0.0;
}

int ProblemTable::get_tested_on_revision(const std::string &username) const {
    Statement statement;

    std::string sql = "SELECT tested_on_revision FROM " + get_table_name() + " WHERE username = ?";

    DataBase::instance().prepare_statement(statement, sql, username);

    if (statement.step() == SQLITE_ROW) {
        return std::stoi(statement.column_text(0));
    }

    return 0;
}

double ProblemTable::get_score_on_testset(const std::string &username, const Testset &testset) const {
    Statement statement;

    std::string sql = "SELECT score_on_" + testset.get_testset_id() + " FROM " + get_table_name() + " WHERE username = ?";

    DataBase::instance().prepare_statement(statement, sql, username);

    if (statement.step() == SQLITE_ROW) {
        return std::stod(statement.column_text(0));
    }

    return 0.0;
}

double ProblemTable::get_score_on_subtask(const std::string &username, const Testset &testset, const Subtask &subtask) const {
    Statement statement;

    std::string sql = "SELECT score_on_" + testset.get_testset_id() + "_" + subtask.get_subtask_id() + " FROM " + get_table_name() + " WHERE username = ?";

    DataBase::instance().prepare_statement(statement, sql, username);

    if (statement.step() == SQLITE_ROW) {
        return std::stod(statement.column_text(0));
    }

    return 0.0;
}

double ProblemTable::get_score_on_test(const std::string &username, const Testset &testset, const Subtask &subtask, const Test &test) const {
    Statement statement;

    std::string sql = "SELECT score_on_" + testset.get_testset_id() + "_" + subtask.get_subtask_id() + "_" + test.get_test_id() + " FROM " + get_table_name() + " WHERE username = ?";

    DataBase::instance().prepare_statement(statement, sql, username);

    if (statement.step() == SQLITE_ROW) {
        return std::stod(statement.column_text(0));
    }

    return 0.0;
}

bool ProblemTable::does_user_have_submission(const std::string &username) const {
    Statement statement;

    std::string sql = "SELECT 1 FROM " + get_table_name() + " WHERE username = ?";

    DataBase::instance().prepare_statement(statement, sql, username);

    if (statement.step() == SQLITE_ROW) {
        return true;
    }

    return false;
}


std::vector<std::string> ProblemTable::get_right_columns() const {
    std::vector<std::string> columns = {"username", "total_score", "tested_on_revision"};

    for (auto testset : _config.get_testsets()) {
        columns.push_back("score_on_" + testset->get_testset_id());
    }

    for (auto testset : _config.get_testsets()) {
        for (auto subtask : testset->get_subtasks()) {
            columns.push_back("score_on_" + testset->get_testset_id() + "_" + subtask->get_subtask_id());
        }
    }

    for (auto testset : _config.get_testsets()) {
        for (auto subtask : testset->get_subtasks()) {
            for (auto test : subtask->get_tests()) {
                columns.push_back("score_on_" + testset->get_testset_id() + "_" + subtask->get_subtask_id() + "_" + test->get_test_id());
            }
        }
    }

    return columns;
}

void ProblemTable::touch_table() {
    std::vector<std::string> columns = get_right_columns();

    Statement statement;
    std::string create_sql = "CREATE TABLE IF NOT EXISTS " + get_table_name() + " (\n";
    bool need_comma = false;
    for (auto &column : columns) {
        if (need_comma) {
            create_sql += ",\n";
        }
        create_sql += column + " ";
        if (column == "username") {
            create_sql += "TEXT PRIMARY KEY";
        } else if (column == "tested_on_revision") {
            create_sql += "INTEGER DEFAULT 0";
        } else {
            create_sql += "REAL DEFAULT 0";
        }
        need_comma = true;
    }
    create_sql += ");";

    DataBase::instance().prepare_statement(statement, create_sql);
    statement.step();
}

} // namespace oink_judge::backend::management