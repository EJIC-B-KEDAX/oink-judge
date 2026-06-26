#include "oink_judge/database/table_submissions.h"

#include "oink_judge/database/connection_pool.h"
#include "oink_judge/database/query.h"

#include <oink_judge/logger/logger.h>

#include <iomanip>
#include <sstream>

namespace oink_judge::database {

namespace {

constexpr const char* K_LOG_MODULE = "database";

using logger::logDebug;

auto firstRow(const QueryResult& result) -> std::optional<QueryRow> {
    if (result.empty()) {
        return std::nullopt;
    }
    return result[0];
}

auto parseTimestamp(const std::string& value) -> std::chrono::system_clock::time_point {
    std::tm tm_value{};
    std::istringstream stream(value);
    stream >> std::get_time(&tm_value, "%Y-%m-%d %H:%M:%S");
    if (stream.fail()) {
        return std::chrono::system_clock::now();
    }
    return std::chrono::system_clock::from_time_t(std::mktime(&tm_value));
}

} // namespace

auto TableSubmissions::instance() -> TableSubmissions& {
    static TableSubmissions table;
    return table;
}

auto TableSubmissions::requireInitialized() -> awaitable<bool> {
    if (!initialized_) {
        co_return co_await initialize();
    }
    co_return true;
}

auto TableSubmissions::initialize() -> awaitable<bool> {
    if (initialized_) {
        logDebug(K_LOG_MODULE, "Submissions table already initialized");
        co_return true;
    }

    try {
        logDebug(K_LOG_MODULE, "Initializing submissions table");
        auto& pool = ConnectionPool::instance();
        co_await pool.initialize();

        const std::string create_sql = "CREATE TABLE IF NOT EXISTS submissions ("
                                       "id TEXT PRIMARY KEY,"
                                       "username TEXT,"
                                       "problem_id TEXT,"
                                       "language TEXT,"
                                       "verdict_type TEXT,"
                                       "score REAL,"
                                       "send_time TIMESTAMP WITHOUT TIME ZONE DEFAULT CURRENT_TIMESTAMP);";

        co_await executeSQL(pool, create_sql);

        pool.prepareStatement("submissions__select_whose", "SELECT username FROM submissions WHERE id = $1");
        pool.prepareStatement("submissions__select_problem", "SELECT problem_id FROM submissions WHERE id = $1");
        pool.prepareStatement("submissions__select_language", "SELECT language FROM submissions WHERE id = $1");
        pool.prepareStatement("submissions__select_verdict_type", "SELECT verdict_type FROM submissions WHERE id = $1");
        pool.prepareStatement("submissions__select_score", "SELECT score FROM submissions WHERE id = $1");
        pool.prepareStatement("submissions__select_by_user_problem",
                              "SELECT id, username, problem_id, language, verdict_type, score, send_time "
                              "FROM submissions WHERE username = $1 AND problem_id = $2 ORDER BY send_time DESC");
        pool.prepareStatement("submissions__insert",
                              "INSERT INTO submissions (id, username, problem_id, language, verdict_type, score, send_time) "
                              "VALUES ($1, $2, $3, $4, $5, $6, to_timestamp($7))");
        pool.prepareStatement("submissions__update_verdict_type", "UPDATE submissions SET verdict_type = $2 WHERE id = $1");
        pool.prepareStatement("submissions__update_score", "UPDATE submissions SET score = $2 WHERE id = $1");
        pool.prepareStatement("submissions__update_verdict_and_score",
                              "UPDATE submissions SET verdict_type = $2, score = $3 WHERE id = $1");

        initialized_ = true;
        logDebug(K_LOG_MODULE, "Submissions table initialized");
        co_return true;
    } catch (...) {
        co_return false;
    }
}

auto TableSubmissions::addSubmission(const SubmissionRow& row) -> awaitable<bool> { // NOLINT
    if (!co_await requireInitialized()) {
        co_return false;
    }

    try {
        logDebug(K_LOG_MODULE, "Adding submission: id=" + row.id + ", user=" + row.username + ", problem=" + row.problem_id, 2);
        const auto timestamp = std::chrono::system_clock::to_time_t(row.send_time);
        co_await execute(ConnectionPool::instance(), "submissions__insert", row.id, row.username, row.problem_id, row.language,
                         row.verdict_type, row.score, static_cast<std::int64_t>(timestamp));
        co_return true;
    } catch (...) {
        co_return false;
    }
}

auto TableSubmissions::loadSubmissionsByUserAndProblem(std::string username, std::string problem_id) // NOLINT
    -> awaitable<std::optional<std::vector<SubmissionRow>>> {
    if (!co_await requireInitialized()) {
        co_return std::nullopt;
    }

    try {
        logDebug(K_LOG_MODULE, "Loading submissions for user=" + username + ", problem=" + problem_id, 2);
        const auto result =
            co_await executeReadOnly(ConnectionPool::instance(), "submissions__select_by_user_problem", username, problem_id);

        std::vector<SubmissionRow> rows;
        rows.reserve(result.size());
        for (std::size_t index = 0; index < result.size(); ++index) {
            const auto row = result[static_cast<int>(index)];
            rows.push_back(SubmissionRow{
                .id = row["id"].as<std::string>(),
                .username = row["username"].as<std::string>(),
                .problem_id = row["problem_id"].as<std::string>(),
                .language = row["language"].as<std::string>(),
                .verdict_type = row["verdict_type"].as<std::string>(),
                .score = row["score"].as<double>(),
                .send_time = parseTimestamp(row["send_time"].as<std::string>()),
            });
        }
        logDebug(K_LOG_MODULE, "Loaded " + std::to_string(rows.size()) + " submission(s)", 2);
        co_return rows;
    } catch (...) {
        co_return std::nullopt;
    }
}

auto TableSubmissions::updateSubmissionVerdict(std::string submission_id, std::string verdict_type, double score) // NOLINT
    -> awaitable<bool> {
    if (!co_await requireInitialized()) {
        co_return false;
    }

    try {
        logDebug(K_LOG_MODULE, "Updating submission verdict: id=" + submission_id + ", verdict=" + verdict_type, 2);
        co_await execute(ConnectionPool::instance(), "submissions__update_verdict_and_score", submission_id, verdict_type,
                         score);
        co_return true;
    } catch (...) {
        co_return false;
    }
}

auto TableSubmissions::whoseSubmission(std::string submission_id) -> awaitable<std::optional<std::string>> { // NOLINT
    if (!co_await requireInitialized()) {
        co_return std::nullopt;
    }

    try {
        const auto result = co_await executeReadOnly(ConnectionPool::instance(), "submissions__select_whose", submission_id);
        const auto row = firstRow(result);
        if (!row) {
            co_return std::nullopt;
        }
        co_return (*row)[0].as<std::string>();
    } catch (...) {
        co_return std::nullopt;
    }
}

auto TableSubmissions::problemOfSubmission(std::string submission_id) -> awaitable<std::optional<std::string>> { // NOLINT
    if (!co_await requireInitialized()) {
        co_return std::nullopt;
    }

    try {
        const auto result = co_await executeReadOnly(ConnectionPool::instance(), "submissions__select_problem", submission_id);
        const auto row = firstRow(result);
        if (!row) {
            co_return std::nullopt;
        }
        co_return (*row)[0].as<std::string>();
    } catch (...) {
        co_return std::nullopt;
    }
}

auto TableSubmissions::languageOfSubmission(std::string submission_id) -> awaitable<std::optional<std::string>> { // NOLINT
    if (!co_await requireInitialized()) {
        co_return std::nullopt;
    }

    try {
        const auto result = co_await executeReadOnly(ConnectionPool::instance(), "submissions__select_language", submission_id);
        const auto row = firstRow(result);
        if (!row) {
            co_return std::nullopt;
        }
        co_return (*row)[0].as<std::string>();
    } catch (...) {
        co_return std::nullopt;
    }
}

auto TableSubmissions::verdictTypeOfSubmission(std::string submission_id) -> awaitable<std::optional<std::string>> { // NOLINT
    if (!co_await requireInitialized()) {
        co_return std::nullopt;
    }

    try {
        const auto result =
            co_await executeReadOnly(ConnectionPool::instance(), "submissions__select_verdict_type", submission_id);
        const auto row = firstRow(result);
        if (!row) {
            co_return std::nullopt;
        }
        co_return (*row)[0].as<std::string>();
    } catch (...) {
        co_return std::nullopt;
    }
}

auto TableSubmissions::scoreOfSubmission(std::string submission_id) -> awaitable<std::optional<double>> { // NOLINT
    if (!co_await requireInitialized()) {
        co_return std::nullopt;
    }

    try {
        const auto result = co_await executeReadOnly(ConnectionPool::instance(), "submissions__select_score", submission_id);
        const auto row = firstRow(result);
        if (!row) {
            co_return std::nullopt;
        }
        co_return (*row)[0].as<double>();
    } catch (...) {
        co_return std::nullopt;
    }
}

auto TableSubmissions::setVerdictType(std::string submission_id, std::string verdict_type) -> awaitable<bool> { // NOLINT
    if (!co_await requireInitialized()) {
        co_return false;
    }

    try {
        co_await execute(ConnectionPool::instance(), "submissions__update_verdict_type", submission_id, verdict_type);
        co_return true;
    } catch (...) {
        co_return false;
    }
}

auto TableSubmissions::setScore(std::string submission_id, double score) -> awaitable<bool> { // NOLINT
    if (!co_await requireInitialized()) {
        co_return false;
    }

    try {
        co_await execute(ConnectionPool::instance(), "submissions__update_score", submission_id, score);
        co_return true;
    } catch (...) {
        co_return false;
    }
}

} // namespace oink_judge::database
