#pragma once

#include <string>
#include "ParameterizedTypeFactory.h"

namespace oink_judge::services::dispatcher {

class ProblemSubmissionManager {
public:
    virtual ~ProblemSubmissionManager() = default;

    virtual void handle_submission(const std::string &submission_id) = 0;
};

using ProblemSubmissionManagerFactory = ParameterizedTypeFactory<std::shared_ptr<ProblemSubmissionManager>, const std::string&>;

} // namespace oink_judge::services::dispatcher
