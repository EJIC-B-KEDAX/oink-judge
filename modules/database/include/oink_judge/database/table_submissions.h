#pragma once
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

    auto initialize() -> awaitable<bool>;

    auto addSubmission(const SubmissionRow& row) -> awaitable<bool>;
    auto loadSubmissionsByUserAndProblem(std::string username, std::string problem_id)
        -> awaitable<std::optional<std::vector<SubmissionRow>>>;
    auto updateSubmissionVerdict(std::string submission_id, std::string verdict_type, double score) -> awaitable<bool>;

    auto whoseSubmission(std::string submission_id) -> awaitable<std::optional<std::string>>;
    auto problemOfSubmission(std::string submission_id) -> awaitable<std::optional<std::string>>;
    auto languageOfSubmission(std::string submission_id) -> awaitable<std::optional<std::string>>;
    auto verdictTypeOfSubmission(std::string submission_id) -> awaitable<std::optional<std::string>>;
    auto scoreOfSubmission(std::string submission_id) -> awaitable<std::optional<double>>;

    auto setVerdictType(std::string submission_id, std::string verdict_type) -> awaitable<bool>;
    auto setScore(std::string submission_id, double score) -> awaitable<bool>;

  private:
    TableSubmissions() = default;

    auto requireInitialized() -> awaitable<bool>;

    bool initialized_ = false;
};

} // namespace oink_judge::database
