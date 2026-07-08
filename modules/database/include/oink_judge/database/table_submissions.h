#pragma once
#include "oink_judge/database/table_execute_options.h"

#include <boost/asio/awaitable.hpp>

#include <chrono>
#include <optional>
#include <string>
#include <vector>

namespace oink_judge::database {

using boost::asio::awaitable;

struct SubmissionRow {
    std::string id;
    std::string username;
    std::string problem_id;
    std::string language;
    std::string verdict_type;
    double score = 0.0;
    std::chrono::system_clock::time_point send_time;
};

class TableSubmissions {
  public:
    static auto instance() -> TableSubmissions&;

    auto initialize(TableExecuteOptions options = TableExecuteOptions{}) -> awaitable<bool>;

    auto addSubmission(TableExecuteOptions options, const SubmissionRow& row) -> awaitable<bool>;
    auto addSubmission(const SubmissionRow& row) -> awaitable<bool>;

    auto loadSubmissionsByUserAndProblem(TableExecuteOptions options, std::string username, std::string problem_id)
        -> awaitable<std::optional<std::vector<SubmissionRow>>>;
    auto loadSubmissionsByUserAndProblem(std::string username, std::string problem_id)
        -> awaitable<std::optional<std::vector<SubmissionRow>>>;

    auto updateSubmissionVerdict(TableExecuteOptions options, std::string submission_id, std::string verdict_type, double score)
        -> awaitable<bool>;
    auto updateSubmissionVerdict(std::string submission_id, std::string verdict_type, double score) -> awaitable<bool>;

    auto whoseSubmission(TableExecuteOptions options, std::string submission_id) -> awaitable<std::optional<std::string>>;
    auto whoseSubmission(std::string submission_id) -> awaitable<std::optional<std::string>>;

    auto problemOfSubmission(TableExecuteOptions options, std::string submission_id) -> awaitable<std::optional<std::string>>;
    auto problemOfSubmission(std::string submission_id) -> awaitable<std::optional<std::string>>;

    auto languageOfSubmission(TableExecuteOptions options, std::string submission_id) -> awaitable<std::optional<std::string>>;
    auto languageOfSubmission(std::string submission_id) -> awaitable<std::optional<std::string>>;

    auto verdictTypeOfSubmission(TableExecuteOptions options, std::string submission_id) -> awaitable<std::optional<std::string>>;
    auto verdictTypeOfSubmission(std::string submission_id) -> awaitable<std::optional<std::string>>;

    auto scoreOfSubmission(TableExecuteOptions options, std::string submission_id) -> awaitable<std::optional<double>>;
    auto scoreOfSubmission(std::string submission_id) -> awaitable<std::optional<double>>;

    auto setVerdictType(TableExecuteOptions options, std::string submission_id, std::string verdict_type) -> awaitable<bool>;
    auto setVerdictType(std::string submission_id, std::string verdict_type) -> awaitable<bool>;

    auto setScore(TableExecuteOptions options, std::string submission_id, double score) -> awaitable<bool>;
    auto setScore(std::string submission_id, double score) -> awaitable<bool>;

    static constexpr auto STATEMENTS_BLOCK_NAME = "table_submissions__initialize";

  private:
    TableSubmissions() = default;

    auto requireInitialized(TableExecuteOptions options = TableExecuteOptions{}) -> awaitable<bool>;
};

} // namespace oink_judge::database
