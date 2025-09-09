#pragma once

#include "ProblemSubmissionManager.hpp"

namespace oink_judge::services::dispatcher {

class BasicProblemSubmissionManager : public ProblemSubmissionManager {
public:
    BasicProblemSubmissionManager(const std::string &problem_id);

    void handle_submission(const std::string &submission_id) override;

    constexpr static auto REGISTERED_NAME = "BasicProblemSubmissionManager";

private:
    std::string _problem_id;
};

} // namespace oink_judge::services::dispatcher
