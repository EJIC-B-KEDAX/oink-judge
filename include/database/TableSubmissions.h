#pragma once
#include <string>

namespace oink_judge::database {

class TableSubmissions {
public:
    static TableSubmissions &instance();

    TableSubmissions(const TableSubmissions &) = delete;
    TableSubmissions &operator=(const TableSubmissions &) = delete;

    std::string whose_submission(const std::string &submission_id) const;
    std::string problem_of_submission(const std::string &submission_id) const;
    std::string language_of_submission(const std::string &submission_id) const;
    std::string verdict_type_of_submission(const std::string &submission_id) const;
    double score_of_submission(const std::string &submission_id) const;

    void set_verdict_type(const std::string &submission_id, const std::string &verdict_type);
    void set_score(const std::string &submission_id, double score);

private:
    TableSubmissions();
};

} // namespace oink_judge::database
