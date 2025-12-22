#include "database/TableSubmissions.h"
#include "database/DataBase.h"

namespace oink_judge::database {

TableSubmissions &TableSubmissions::instance() {
    static TableSubmissions instance;
    return instance;
}

std::string TableSubmissions::whose_submission(const std::string &submission_id) const {
    pqxx::result res = DataBase::instance().execute_read_only("submissions__select_whose", submission_id);

    if (res.empty()) {
        throw std::runtime_error("Submission not found: " + submission_id);
    }

    return res[0][0].as<std::string>();
}

std::string TableSubmissions::problem_of_submission(const std::string &submission_id) const {
    pqxx::result res = DataBase::instance().execute_read_only("submissions__select_problem", submission_id);

    if (res.empty()) {
        throw std::runtime_error("Submission not found: " + submission_id);
    }

    return res[0][0].as<std::string>();
}

std::string TableSubmissions::language_of_submission(const std::string &submission_id) const {
    pqxx::result res = DataBase::instance().execute_read_only("submissions__select_language", submission_id);

    if (res.empty()) {
        throw std::runtime_error("Submission not found: " + submission_id);
    }

    return res[0][0].as<std::string>();
}

std::string TableSubmissions::verdict_type_of_submission(const std::string &submission_id) const {
    pqxx::result res = DataBase::instance().execute_read_only("submissions__select_verdict_type", submission_id);

    if (res.empty()) {
        throw std::runtime_error("Submission not found: " + submission_id);
    }

    return res[0][0].as<std::string>();
}

double TableSubmissions::score_of_submission(const std::string &submission_id) const {
    pqxx::result res = DataBase::instance().execute_read_only("submissions__select_score", submission_id);

    if (res.empty()) {
        throw std::runtime_error("Submission not found: " + submission_id);
    }

    return res[0][0].as<double>();
}

void TableSubmissions::set_verdict_type(const std::string &submission_id, const std::string &verdict_type) {
    DataBase::instance().execute("submissions__update_verdict_type", submission_id, verdict_type);
}

void TableSubmissions::set_score(const std::string &submission_id, double score) {
    DataBase::instance().execute("submissions__update_score", submission_id, score);
}

TableSubmissions::TableSubmissions() {
    const std::string create_sql = "CREATE TABLE IF NOT EXISTS submissions ("
                                       "id TEXT PRIMARY KEY,"
                                       "username TEXT,"
                                       "problem_id TEXT,"
                                       "language TEXT,"
                                       "verdict_type TEXT,"
                                       "score REAL,"
                                       "send_time TIMESTAMP WITHOUT TIME ZONE DEFAULT CURRENT_TIMESTAMP);";

    DataBase::instance().execute_sql(create_sql);

    const std::string select_whose_sql = "SELECT username FROM submissions WHERE id = $1";
    const std::string select_problem_sql = "SELECT problem_id FROM submissions WHERE id = $1";
    const std::string select_language_sql = "SELECT language FROM submissions WHERE id = $1";
    const std::string select_verdict_type_sql = "SELECT verdict_type FROM submissions WHERE id = $1";
    const std::string select_score_sql = "SELECT score FROM submissions WHERE id = $1";

    const std::string update_verdict_type_sql = "UPDATE submissions SET verdict_type = $2 WHERE id = $1";
    const std::string update_score_sql = "UPDATE submissions SET score = $2 WHERE id = $1";

    DataBase::instance().prepare_statement("submissions__select_whose", select_whose_sql);
    DataBase::instance().prepare_statement("submissions__select_problem", select_problem_sql);
    DataBase::instance().prepare_statement("submissions__select_language", select_language_sql);
    DataBase::instance().prepare_statement("submissions__select_verdict_type", select_verdict_type_sql);
    DataBase::instance().prepare_statement("submissions__select_score", select_score_sql);

    DataBase::instance().prepare_statement("submissions__update_verdict_type", update_verdict_type_sql);
    DataBase::instance().prepare_statement("submissions__update_score", update_score_sql);
}

} // namespace oink_judge::database
