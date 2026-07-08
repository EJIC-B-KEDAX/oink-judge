#include "oink_judge/database/table_submissions.h"

#include "oink_judge/database/database_executor_interface.h"
#include "oink_judge/database/query.h"
#include "oink_judge/database/statements.h"

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

auto TableSubmissions::requireInitialized(TableExecuteOptions options) -> awaitable<bool> {
    options.fillWithDefaults();
    if (!options.executor->isPrepared(STATEMENTS_BLOCK_NAME)) {
        co_return co_await initialize(options);
    }
    co_return true;
}

auto TableSubmissions::initialize(TableExecuteOptions options) -> awaitable<bool> { // NOLINT
    options.fillWithDefaults();
    auto executor = options.executor;

    if (executor->isPrepared(STATEMENTS_BLOCK_NAME)) {
        logDebug(K_LOG_MODULE, "Submissions table already initialized", 2);
        co_return true;
    }

    try {
        logDebug(K_LOG_MODULE, "Initializing submissions table");

        const std::string create_sql = "CREATE TABLE IF NOT EXISTS submissions ("
                                       "id TEXT PRIMARY KEY,"
                                       "username TEXT,"
                                       "problem_id TEXT,"
                                       "language TEXT,"
                                       "verdict_type TEXT,"
                                       "score REAL,"
                                       "send_time TIMESTAMP WITHOUT TIME ZONE DEFAULT CURRENT_TIMESTAMP);";

        co_await executor->executeSQL(options.execute_options, create_sql);

        StatementsBlock statements_block(STATEMENTS_BLOCK_NAME);

        statements_block.addStatement({"submissions__select_whose", "SELECT username FROM submissions WHERE id = $1"});
        statements_block.addStatement({"submissions__select_problem", "SELECT problem_id FROM submissions WHERE id = $1"});
        statements_block.addStatement({"submissions__select_language", "SELECT language FROM submissions WHERE id = $1"});
        statements_block.addStatement({"submissions__select_verdict_type", "SELECT verdict_type FROM submissions WHERE id = $1"});
        statements_block.addStatement({"submissions__select_score", "SELECT score FROM submissions WHERE id = $1"});
        statements_block.addStatement({"submissions__select_by_user_problem",
                                       "SELECT id, username, problem_id, language, verdict_type, score, send_time "
                                       "FROM submissions WHERE username = $1 AND problem_id = $2 ORDER BY send_time DESC"});
        statements_block.addStatement(
            {"submissions__insert",
             "INSERT INTO submissions (id, username, problem_id, language, verdict_type, score, send_time) "
             "VALUES ($1, $2, $3, $4, $5, $6, to_timestamp($7))"});
        statements_block.addStatement(
            {"submissions__update_verdict_type", "UPDATE submissions SET verdict_type = $2 WHERE id = $1"});
        statements_block.addStatement({"submissions__update_score", "UPDATE submissions SET score = $2 WHERE id = $1"});
        statements_block.addStatement(
            {"submissions__update_verdict_and_score", "UPDATE submissions SET verdict_type = $2, score = $3 WHERE id = $1"});

        co_await executor->prepareStatements(std::move(statements_block));
        logDebug(K_LOG_MODULE, "Submissions table initialized");
        co_return true;
    } catch (...) {
        co_return false;
    }
}

auto TableSubmissions::addSubmission(TableExecuteOptions options, const SubmissionRow& row) -> awaitable<bool> { // NOLINT
    options.fillWithDefaults();
    if (!co_await requireInitialized(options)) {
        co_return false;
    }
    auto executor = options.executor;

    try {
        logDebug(K_LOG_MODULE, "Adding submission: id=" + row.id + ", user=" + row.username + ", problem=" + row.problem_id, 2);
        const auto timestamp = std::chrono::system_clock::to_time_t(row.send_time);
        co_await executor->execute(options.execute_options, "submissions__insert", row.id, row.username, row.problem_id,
                                   row.language, row.verdict_type, row.score, static_cast<std::int64_t>(timestamp));
        co_return true;
    } catch (...) {
        co_return false;
    }
}

auto TableSubmissions::addSubmission(const SubmissionRow& row) -> awaitable<bool> { // NOLINT
    return addSubmission(TableExecuteOptions{}, row);
}

auto TableSubmissions::loadSubmissionsByUserAndProblem(TableExecuteOptions options, std::string username,
                                                       std::string problem_id) // NOLINT
    -> awaitable<std::optional<std::vector<SubmissionRow>>> {
    options.fillWithDefaults();
    if (!co_await requireInitialized(options)) {
        co_return std::nullopt;
    }
    auto executor = options.executor;

    try {
        logDebug(K_LOG_MODULE, "Loading submissions for user=" + username + ", problem=" + problem_id, 2);
        auto execute_options = options.execute_options;
        execute_options.read_only = true;
        const auto result =
            co_await executor->execute(execute_options, "submissions__select_by_user_problem", username, problem_id);

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

auto TableSubmissions::loadSubmissionsByUserAndProblem(std::string username, std::string problem_id)
    -> awaitable<std::optional<std::vector<SubmissionRow>>> {
    return loadSubmissionsByUserAndProblem(TableExecuteOptions{}, std::move(username), std::move(problem_id));
}
auto TableSubmissions::updateSubmissionVerdict(TableExecuteOptions options, std::string submission_id, std::string verdict_type,
                                               double score) // NOLINT
    -> awaitable<bool> {
    options.fillWithDefaults();
    if (!co_await requireInitialized(options)) {
        co_return false;
    }
    auto executor = options.executor;

    try {
        logDebug(K_LOG_MODULE, "Updating submission verdict: id=" + submission_id + ", verdict=" + verdict_type, 2);
        co_await executor->execute(options.execute_options, "submissions__update_verdict_and_score", submission_id, verdict_type,
                                   score);
        co_return true;
    } catch (...) {
        co_return false;
    }
}

auto TableSubmissions::updateSubmissionVerdict(std::string submission_id, std::string verdict_type, double score)
    -> awaitable<bool> {
    return updateSubmissionVerdict(TableExecuteOptions{}, std::move(submission_id), std::move(verdict_type), score);
}

auto TableSubmissions::whoseSubmission(TableExecuteOptions options, std::string submission_id)
    -> awaitable<std::optional<std::string>> { // NOLINT
    options.fillWithDefaults();
    if (!co_await requireInitialized(options)) {
        co_return std::nullopt;
    }
    auto executor = options.executor;

    try {
        auto execute_options = options.execute_options;
        execute_options.read_only = true;
        const auto result = co_await executor->execute(execute_options, "submissions__select_whose", submission_id);
        const auto row = firstRow(result);
        if (!row) {
            co_return std::nullopt;
        }
        co_return (*row)[0].as<std::string>();
    } catch (...) {
        co_return std::nullopt;
    }
}

auto TableSubmissions::whoseSubmission(std::string submission_id) -> awaitable<std::optional<std::string>> {
    return whoseSubmission(TableExecuteOptions{}, std::move(submission_id));
}

auto TableSubmissions::problemOfSubmission(TableExecuteOptions options, std::string submission_id)
    -> awaitable<std::optional<std::string>> { // NOLINT
    options.fillWithDefaults();
    if (!co_await requireInitialized(options)) {
        co_return std::nullopt;
    }
    auto executor = options.executor;

    try {
        auto execute_options = options.execute_options;
        execute_options.read_only = true;
        const auto result = co_await executor->execute(execute_options, "submissions__select_problem", submission_id);
        const auto row = firstRow(result);
        if (!row) {
            co_return std::nullopt;
        }
        co_return (*row)[0].as<std::string>();
    } catch (...) {
        co_return std::nullopt;
    }
}

auto TableSubmissions::problemOfSubmission(std::string submission_id) -> awaitable<std::optional<std::string>> {
    return problemOfSubmission(TableExecuteOptions{}, std::move(submission_id));
}

auto TableSubmissions::languageOfSubmission(TableExecuteOptions options, std::string submission_id)
    -> awaitable<std::optional<std::string>> { // NOLINT
    options.fillWithDefaults();
    if (!co_await requireInitialized(options)) {
        co_return std::nullopt;
    }
    auto executor = options.executor;

    try {
        auto execute_options = options.execute_options;
        execute_options.read_only = true;
        const auto result = co_await executor->execute(execute_options, "submissions__select_language", submission_id);
        const auto row = firstRow(result);
        if (!row) {
            co_return std::nullopt;
        }
        co_return (*row)[0].as<std::string>();
    } catch (...) {
        co_return std::nullopt;
    }
}

auto TableSubmissions::languageOfSubmission(std::string submission_id) -> awaitable<std::optional<std::string>> {
    return languageOfSubmission(TableExecuteOptions{}, std::move(submission_id));
}

auto TableSubmissions::verdictTypeOfSubmission(TableExecuteOptions options, std::string submission_id)
    -> awaitable<std::optional<std::string>> { // NOLINT
    options.fillWithDefaults();
    if (!co_await requireInitialized(options)) {
        co_return std::nullopt;
    }
    auto executor = options.executor;
    try {
        auto execute_options = options.execute_options;
        execute_options.read_only = true;
        const auto result = co_await executor->execute(execute_options, "submissions__select_verdict_type", submission_id);
        const auto row = firstRow(result);
        if (!row) {
            co_return std::nullopt;
        }
        co_return (*row)[0].as<std::string>();
    } catch (...) {
        co_return std::nullopt;
    }
}

auto TableSubmissions::verdictTypeOfSubmission(std::string submission_id) -> awaitable<std::optional<std::string>> {
    return verdictTypeOfSubmission(TableExecuteOptions{}, std::move(submission_id));
}

auto TableSubmissions::scoreOfSubmission(TableExecuteOptions options, std::string submission_id)
    -> awaitable<std::optional<double>> { // NOLINT
    options.fillWithDefaults();
    if (!co_await requireInitialized(options)) {
        co_return std::nullopt;
    }
    auto executor = options.executor;

    try {
        auto execute_options = options.execute_options;
        execute_options.read_only = true;
        const auto result = co_await executor->execute(execute_options, "submissions__select_score", submission_id);
        const auto row = firstRow(result);
        if (!row) {
            co_return std::nullopt;
        }
        co_return (*row)[0].as<double>();
    } catch (...) {
        co_return std::nullopt;
    }
}

auto TableSubmissions::scoreOfSubmission(std::string submission_id) -> awaitable<std::optional<double>> {
    return scoreOfSubmission(TableExecuteOptions{}, std::move(submission_id));
}

auto TableSubmissions::setVerdictType(TableExecuteOptions options, std::string submission_id, std::string verdict_type)
    -> awaitable<bool> { // NOLINT
    options.fillWithDefaults();
    if (!co_await requireInitialized(options)) {
        co_return false;
    }
    auto executor = options.executor;

    try {
        co_await executor->execute(options.execute_options, "submissions__update_verdict_type", submission_id, verdict_type);
        co_return true;
    } catch (...) {
        co_return false;
    }
}

auto TableSubmissions::setVerdictType(std::string submission_id, std::string verdict_type) -> awaitable<bool> {
    return setVerdictType(TableExecuteOptions{}, std::move(submission_id), std::move(verdict_type));
}

auto TableSubmissions::setScore(TableExecuteOptions options, std::string submission_id, double score)
    -> awaitable<bool> { // NOLINT
    options.fillWithDefaults();
    if (!co_await requireInitialized(options)) {
        co_return false;
    }
    auto executor = options.executor;

    try {
        co_await executor->execute(options.execute_options, "submissions__update_score", submission_id, score);
        co_return true;
    } catch (...) {
        co_return false;
    }
}

auto TableSubmissions::setScore(std::string submission_id, double score) -> awaitable<bool> {
    return setScore(TableExecuteOptions{}, std::move(submission_id), score);
}

} // namespace oink_judge::database
