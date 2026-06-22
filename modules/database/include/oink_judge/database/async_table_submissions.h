#pragma once
#include <boost/asio/awaitable.hpp>

#include <chrono>
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

class AsyncTableSubmissions {
  public:
    static auto instance() -> AsyncTableSubmissions&;

    auto initialize() -> awaitable<void>;

    auto addSubmission(const SubmissionRow& row) -> awaitable<void>;
    auto loadSubmissionsByUserAndProblem(std::string username, std::string problem_id) -> awaitable<std::vector<SubmissionRow>>;
    auto updateSubmissionVerdict(std::string submission_id, std::string verdict_type, double score) -> awaitable<void>;

    auto whoseSubmission(std::string submission_id) -> awaitable<std::string>;
    auto problemOfSubmission(std::string submission_id) -> awaitable<std::string>;
    auto languageOfSubmission(std::string submission_id) -> awaitable<std::string>;
    auto verdictTypeOfSubmission(std::string submission_id) -> awaitable<std::string>;
    auto scoreOfSubmission(std::string submission_id) -> awaitable<double>;

    auto setVerdictType(std::string submission_id, std::string verdict_type) -> awaitable<void>;
    auto setScore(std::string submission_id, double score) -> awaitable<void>;

  private:
    AsyncTableSubmissions() = default;

    auto requireInitialized() -> awaitable<void>;

    bool initialized_ = false;
};

} // namespace oink_judge::database
