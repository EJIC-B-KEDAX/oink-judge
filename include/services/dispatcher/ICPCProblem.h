#pragma once

#include "services/dispatcher/Problem.h"

namespace oink_judge::services::dispatcher {

class ICPCProblem : public Problem {
public:
    explicit ICPCProblem(const std::string &id);

    void handle_submission(const std::string &submission_id) override;
};

} // namespace oink_judge::services::dispatcher
