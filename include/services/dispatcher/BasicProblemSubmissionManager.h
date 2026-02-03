#pragma once
#include "ProblemSubmissionManager.hpp"

namespace oink_judge::services::dispatcher {

class BasicProblemSubmissionManager : public ProblemSubmissionManager { // TODO: rename class
  public:
    BasicProblemSubmissionManager(std::string problem_id);

    void handleSubmission(const std::string& submission_id) override;

    constexpr static auto REGISTERED_NAME = "BasicProblemSubmissionManager";

  private:
    std::string problem_id_;
};

} // namespace oink_judge::services::dispatcher
