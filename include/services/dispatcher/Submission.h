#pragma once
#include <string>

namespace oink_judge::services::dispatcher {

struct SubmissionInfo {
    std::string participant;
    std::string language;
    std::string problem_id;
    time_t sending_time;
};

SubmissionInfo load_submission_info(const std::string &submission_id);
void store_submission_info(const std::string &submission_id, const SubmissionInfo &submission_info);

} // namespace oink_judge::services::dispatcher
