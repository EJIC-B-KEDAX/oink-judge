#include "backend/management/ProblemTable.h"
#include "database/DataBase.h"
#include <cassert>
#include <iostream>

namespace oink_judge::backend::management {

using DataBase = database::DataBase;

namespace {

std::string generate_create_sql(const std::string &table_name, const std::vector<std::string> &colums) {
    std::string create_sql = "CREATE TABLE IF NOT EXISTS " + table_name + " (\n";
    bool need_comma = false;
    for (auto &column : colums) {
        if (need_comma) {
            create_sql += ",\n";
        }
        create_sql += column + " ";
        if (column == DataBase::instance().quote_name("username")) {
            create_sql += "TEXT PRIMARY KEY";
        } else if (column == DataBase::instance().quote_name("tested_on_revision")) {
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

ProblemTable::ProblemTable(const ProblemConfig &problem_config) : _config(problem_config) {
    _id = _config.get_id();
    _short_name = _config.get_short_name();

    touch_table();

    check_table_actuality();

    const std::string set_total_score_sql = "UPDATE " + get_table_name() + " SET total_score = $2 WHERE username = $1";

    const std::string set_tested_on_revision_sql = "UPDATE " + get_table_name() + " SET tested_on_revision = $2 WHERE username = $1";

    const std::string add_new_user_sql = "INSERT INTO " + get_table_name() + " (username) VALUES ($1)";

    const std::string select_user_line_sql = "SELECT 1 FROM " + get_table_name() + " WHERE username = $1";

    const std::string get_total_score_sql = "SELECT total_score FROM " + get_table_name() + " WHERE username = $1";

    const std::string get_tested_on_revision_sql = "SELECT tested_on_revision FROM " + get_table_name() + " WHERE username = $1";

    DataBase::instance().prepare_statement(get_table_name() + "__set_total_score", set_total_score_sql);
    DataBase::instance().prepare_statement(get_table_name() + "__set_tested_on_revision", set_tested_on_revision_sql);
    DataBase::instance().prepare_statement(get_table_name() + "__add_new_user", add_new_user_sql);
    DataBase::instance().prepare_statement(get_table_name() + "__select_user_line", select_user_line_sql);
    DataBase::instance().prepare_statement(get_table_name() + "__get_total_score", get_total_score_sql);
    DataBase::instance().prepare_statement(get_table_name() + "__get_tested_on_revision", get_tested_on_revision_sql);

    // TODO add statement for set full string
}

bool ProblemTable::check_table_actuality() {
    std::vector<std::string> columns = get_columns();
    std::vector<std::string> right_columns = get_right_columns();

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
        std::string drop_sql = "ALTER TABLE " + get_table_name() + " DROP COLUMN " + columns[i] + ";";

        DataBase::instance().execute_sql(drop_sql);
    }

    for (int i = 3; i < right_columns.size(); i++) {
        std::string add_sql = "ALTER TABLE " + get_table_name() + " ADD COLUMN " + right_columns[i] + " REAL DEFAULT 0;";

        DataBase::instance().execute_sql(add_sql);
    }

    return false;
}

std::string ProblemTable::get_table_name() const {
    return DataBase::instance().quote_table("problem_" + _id + "_" + _short_name);
}

std::string ProblemTable::get_raw_table_name() const {
    return "problem_" + _id + "_" + _short_name;
}

std::vector<std::string> ProblemTable::get_columns() const {
    std::string sql = "SELECT column_name "
                      "FROM information_schema.columns "
                      "WHERE table_name = $1 "
                      "ORDER BY ordinal_position;";

    pqxx::result res = DataBase::instance().execute_sql_read_only(sql, get_raw_table_name());

    std::vector<std::string> columns;

    for (auto row : res) {
        columns.push_back(DataBase::instance().quote_name(row["column_name"].as<std::string>()));
    }

    return columns;
}

void ProblemTable::set_total_score(const std::string &username, double total_score) {
    add_new_user(username);

    DataBase::instance().execute(get_table_name() + "__set_total_score", username, total_score);
}

void ProblemTable::set_tested_on_revision(const std::string &username, int revision) {
    add_new_user(username);

    DataBase::instance().execute(get_table_name() + "__set_tested_on_revision", username, revision);
}

void ProblemTable::set_score_on_testset(const std::string &username, const Testset &testset, double score) {
    add_new_user(username);

    std::string column_name = DataBase::instance().quote_name("score_on_" + testset.get_testset_id());

    std::string sql = "UPDATE " + get_table_name() + " SET " + column_name + " = $2 WHERE username = $1";

    DataBase::instance().execute_sql(sql, username, score);
}

void ProblemTable::set_score_on_subtask(const std::string &username, const Testset &testset, const Subtask &subtask, double score) {
    add_new_user(username);

    std::string column_name = DataBase::instance().quote_name("score_on_" + testset.get_testset_id() + "_" + subtask.get_subtask_id());

    std::string sql = "UPDATE " + get_table_name() + " SET " + column_name + " = $2 WHERE username = $1";

    DataBase::instance().execute_sql(sql, username, score);
}

void ProblemTable::set_score_on_test(const std::string &username, const Testset &testset, const Subtask &subtask, const Test &test, double score) {
    add_new_user(username);

    std::string column_name = DataBase::instance().quote_name("score_on_" + testset.get_testset_id() + "_" + subtask.get_subtask_id() + "_" + test.get_test_id());

    std::string sql = "UPDATE " + get_table_name() + " SET " + column_name + " = $2 WHERE username = $1";

    DataBase::instance().execute_sql(sql, username, score);
}

void ProblemTable::add_new_user(const std::string &username) {
    if (does_user_have_submission(username)) return;

    DataBase::instance().execute(get_table_name() + "__add_new_user", username);
}

double ProblemTable::get_total_score(const std::string &username) const {
    pqxx::result res = DataBase::instance().execute_read_only(get_table_name() + "__get_total_score", username);

    if (res.empty()) return 0.0;

    return res[0]["total_score"].as<double>();
}

int ProblemTable::get_tested_on_revision(const std::string &username) const {
    pqxx::result res = DataBase::instance().execute_read_only(get_table_name() + "__get_tested_on_revision", username);

    if (res.empty()) return 0;

    return res[0]["total_score"].as<int>();
}

double ProblemTable::get_score_on_testset(const std::string &username, const Testset &testset) const {
    std::string column_name = DataBase::instance().quote_name("score_on_" + testset.get_testset_id());

    std::string sql = "SELECT " + column_name + " FROM " + get_table_name() + " WHERE username = $1";

    pqxx::result res = DataBase::instance().execute_sql_read_only(sql, username);

    if (res.empty()) return 0.0;

    return res[0][column_name].as<double>();
}

double ProblemTable::get_score_on_subtask(const std::string &username, const Testset &testset, const Subtask &subtask) const {
    std::string column_name = DataBase::instance().quote_name("score_on_" + testset.get_testset_id() + "_" + subtask.get_subtask_id());

    std::string sql = "SELECT " + column_name + " FROM " + get_table_name() + " WHERE username = $1";

    pqxx::result res = DataBase::instance().execute_sql_read_only(sql, username);

    if (res.empty()) return 0.0;

    return res[0][column_name].as<double>();
}

double ProblemTable::get_score_on_test(const std::string &username, const Testset &testset, const Subtask &subtask, const Test &test) const {
    std::string column_name = DataBase::instance().quote_name("score_on_" + testset.get_testset_id() + "_" + subtask.get_subtask_id() + "_" + test.get_test_id());

    std::string sql = "SELECT " + column_name + " FROM " + get_table_name() + " WHERE username = $1";

    pqxx::result res = DataBase::instance().execute_sql_read_only(sql, username);

    if (res.empty()) return 0.0;

    return res[0][column_name].as<double>();
}

bool ProblemTable::does_user_have_submission(const std::string &username) const {
    pqxx::result res = DataBase::instance().execute_read_only(get_table_name() + "__select_user_line", username);

    return !res.empty();
}


std::vector<std::string> ProblemTable::get_right_columns() const {
    std::vector<std::string> columns = {DataBase::instance().quote_name("username"),
                                        DataBase::instance().quote_name("total_score"),
                                        DataBase::instance().quote_name("tested_on_revision")};

    for (auto testset : _config.get_testsets()) {
        columns.push_back(DataBase::instance().quote_name("score_on_" + testset->get_testset_id()));
    }

    for (auto testset : _config.get_testsets()) {
        for (auto subtask : testset->get_subtasks()) {
            columns.push_back(DataBase::instance().quote_name("score_on_" + testset->get_testset_id() + "_" + subtask->get_subtask_id()));
        }
    }

    for (auto testset : _config.get_testsets()) {
        for (auto subtask : testset->get_subtasks()) {
            for (auto test : subtask->get_tests()) {
                columns.push_back(DataBase::instance().quote_name("score_on_" + testset->get_testset_id() + "_" + subtask->get_subtask_id() + "_" + test->get_test_id()));
            }
        }
    }

    return columns;
}

void ProblemTable::touch_table() {
    DataBase::instance().execute_sql(generate_create_sql(get_table_name(), get_right_columns()));
}

} // namespace oink_judge::backend::management