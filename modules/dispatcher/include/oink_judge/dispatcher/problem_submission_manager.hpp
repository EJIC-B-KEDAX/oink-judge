#pragma once
#include <oink_judge/factory/parameterized_type_factory.hpp>
#include <string>

namespace oink_judge::dispatcher {

class ProblemSubmissionManager {
  public:
    ProblemSubmissionManager(const ProblemSubmissionManager&) = delete;
    auto operator=(const ProblemSubmissionManager&) -> ProblemSubmissionManager& = delete;
    ProblemSubmissionManager(ProblemSubmissionManager&&) = delete;
    auto operator=(ProblemSubmissionManager&&) -> ProblemSubmissionManager& = delete;
    virtual ~ProblemSubmissionManager() = default;

    virtual void handleSubmission(const std::string& submission_id) = 0;

  protected:
    ProblemSubmissionManager() = default;
};

using ProblemSubmissionManagerFactory =
    factory::ParameterizedTypeFactory<std::shared_ptr<ProblemSubmissionManager>, const std::string&>;

} // namespace oink_judge::dispatcher
