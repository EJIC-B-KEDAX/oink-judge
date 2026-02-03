#include "oink_judge/database/table_submissions.h"

#include "oink_judge/database/database.h"

namespace oink_judge::database {

TableSubmissions::~TableSubmissions() {}

auto TableSubmissions::instance() -> TableSubmissions& {
    static TableSubmissions instance;
    return instance;
}

auto TableSubmissions::whoseSubmission(const std::string& submission_id) const -> std::string { // NOLINT
    pqxx::result res = DataBase::instance().executeReadOnly("submissions__select_whose", submission_id);

    if (res.empty()) {
        throw std::runtime_error("Submission not found: " + submission_id);
    }

    return res[0][0].as<std::string>();
}

auto TableSubmissions::problemOfSubmission(const std::string& submission_id) const -> std::string { // NOLINT
    pqxx::result res = DataBase::instance().executeReadOnly("submissions__select_problem", submission_id);

    if (res.empty()) {
        throw std::runtime_error("Submission not found: " + submission_id);
    }

    return res[0][0].as<std::string>();
}

auto TableSubmissions::languageOfSubmission(const std::string& submission_id) const -> std::string { // NOLINT
    pqxx::result res = DataBase::instance().executeReadOnly("submissions__select_language", submission_id);

    if (res.empty()) {
        throw std::runtime_error("Submission not found: " + submission_id);
    }

    return res[0][0].as<std::string>();
}

auto TableSubmissions::verdictTypeOfSubmission(const std::string& submission_id) const -> std::string { // NOLINT
    pqxx::result res = DataBase::instance().executeReadOnly("submissions__select_verdict_type", submission_id);

    if (res.empty()) {
        throw std::runtime_error("Submission not found: " + submission_id);
    }

    return res[0][0].as<std::string>();
}

auto TableSubmissions::scoreOfSubmission(const std::string& submission_id) const -> double { // NOLINT
    pqxx::result res = DataBase::instance().executeReadOnly("submissions__select_score", submission_id);

    if (res.empty()) {
        throw std::runtime_error("Submission not found: " + submission_id);
    }

    return res[0][0].as<double>();
}

auto TableSubmissions::setVerdictType(const std::string& submission_id, const std::string& verdict_type) -> void { // NOLINT
    DataBase::instance().execute("submissions__update_verdict_type", submission_id, verdict_type);
}

auto TableSubmissions::setScore(const std::string& submission_id, double score) -> void { // NOLINT
    DataBase::instance().execute("submissions__update_score", submission_id, score);
}

TableSubmissions::TableSubmissions() {
    const std::string CREATE_SQL = "CREATE TABLE IF NOT EXISTS submissions ("
                                   "id TEXT PRIMARY KEY,"
                                   "username TEXT,"
                                   "problem_id TEXT,"
                                   "language TEXT,"
                                   "verdict_type TEXT,"
                                   "score REAL,"
                                   "send_time TIMESTAMP WITHOUT TIME ZONE DEFAULT CURRENT_TIMESTAMP);";

    DataBase::instance().executeSQL(CREATE_SQL);

    const std::string SELECT_WHOSE_SQL = "SELECT username FROM submissions WHERE id = $1";
    const std::string SELECT_PROBLEM_SQL = "SELECT problem_id FROM submissions WHERE id = $1";
    const std::string SELECT_LANGUAGE_SQL = "SELECT language FROM submissions WHERE id = $1";
    const std::string SELECT_VERDICT_TYPE_SQL = "SELECT verdict_type FROM submissions WHERE id = $1";
    const std::string SELECT_SCORE_SQL = "SELECT score FROM submissions WHERE id = $1";

    const std::string UPDATE_VERDICT_TYPE_SQL = "UPDATE submissions SET verdict_type = $2 WHERE id = $1";
    const std::string UPDATE_SCORE_SQL = "UPDATE submissions SET score = $2 WHERE id = $1";

    DataBase::instance().prepareStatement("submissions__select_whose", SELECT_WHOSE_SQL);
    DataBase::instance().prepareStatement("submissions__select_problem", SELECT_PROBLEM_SQL);
    DataBase::instance().prepareStatement("submissions__select_language", SELECT_LANGUAGE_SQL);
    DataBase::instance().prepareStatement("submissions__select_verdict_type", SELECT_VERDICT_TYPE_SQL);
    DataBase::instance().prepareStatement("submissions__select_score", SELECT_SCORE_SQL);
    DataBase::instance().prepareStatement("submissions__update_verdict_type", UPDATE_VERDICT_TYPE_SQL);
    DataBase::instance().prepareStatement("submissions__update_score", UPDATE_SCORE_SQL);
}

} // namespace oink_judge::database
