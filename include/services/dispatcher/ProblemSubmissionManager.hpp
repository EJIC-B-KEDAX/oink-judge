#pragma once
#include "ParameterizedTypeFactory.hpp"

#include <string>

namespace oink_judge::services::dispatcher {

class ProblemSubmissionManager {
  public:
    virtual ~ProblemSubmissionManager() = default;

    ProblemSubmissionManager(const ProblemSubmissionManager&) = delete;
    auto operator=(const ProblemSubmissionManager&) -> ProblemSubmissionManager& = delete;
    ProblemSubmissionManager(ProblemSubmissionManager&&) = delete;
    auto operator=(ProblemSubmissionManager&&) -> ProblemSubmissionManager& = delete;

    virtual void handleSubmission(const std::string& submission_id) = 0;
};

using ProblemSubmissionManagerFactory = ParameterizedTypeFactory<std::shared_ptr<ProblemSubmissionManager>, const std::string&>;

} // namespace oink_judge::services::dispatcher
