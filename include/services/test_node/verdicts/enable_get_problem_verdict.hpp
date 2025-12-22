#pragma once
#include "services/test_node/Verdict.hpp"

namespace oink_judge::services::test_node {

template<typename T>
concept verdict = std::is_base_of_v<Verdict, T>;

template<verdict verdict_type>
class enable_get_problem_verdict {
public:
    enable_get_problem_verdict() = default;

    std::shared_ptr<verdict_type> get_problem_verdict();
    void set_problem_verdict(std::shared_ptr<verdict_type> verdict);

private:
    std::shared_ptr<verdict_type> problem_verdict = nullptr;
};

} // namespace oink_judge::services::test_node

#include "enable_get_problem_verdict.inl"
