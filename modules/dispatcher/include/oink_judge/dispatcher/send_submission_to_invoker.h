#pragma once
#include "oink_judge/dispatcher/problem_submission_manager.hpp"

namespace oink_judge::dispatcher {

class SendSubmissionToInvoker : public ProblemSubmissionManager {
  public:
    SendSubmissionToInvoker(std::string problem_id);

    void handleSubmission(const std::string& submission_id) override;

    constexpr static auto REGISTERED_NAME = "SendSubmissionToInvoker";

  private:
    std::string problem_id_;
};

} // namespace oink_judge::dispatcher
