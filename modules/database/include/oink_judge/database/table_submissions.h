#pragma once
#include <string>

namespace oink_judge::database {

class TableSubmissions {
  public:
    TableSubmissions(const TableSubmissions&) = delete;
    auto operator=(const TableSubmissions&) -> TableSubmissions& = delete;
    TableSubmissions(TableSubmissions&&) = delete;
    auto operator=(TableSubmissions&&) -> TableSubmissions& = delete;
    ~TableSubmissions();

    static auto instance() -> TableSubmissions&;

    [[nodiscard]] auto whoseSubmission(const std::string& submission_id) const -> std::string;
    [[nodiscard]] auto problemOfSubmission(const std::string& submission_id) const -> std::string;
    [[nodiscard]] auto languageOfSubmission(const std::string& submission_id) const -> std::string;
    [[nodiscard]] auto verdictTypeOfSubmission(const std::string& submission_id) const -> std::string;
    [[nodiscard]] auto scoreOfSubmission(const std::string& submission_id) const -> double;

    auto setVerdictType(const std::string& submission_id, const std::string& verdict_type) -> void;
    auto setScore(const std::string& submission_id, double score) -> void;

  private:
    TableSubmissions();
};

} // namespace oink_judge::database
