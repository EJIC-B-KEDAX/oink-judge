#pragma once
#include "enable_get_problem_verdict.hpp"

namespace oink_judge::services::test_node {

template<verdict verdict_type>
std::shared_ptr<verdict_type> enable_get_problem_verdict<verdict_type>::get_problem_verdict() {
    return problem_verdict;
}

template<verdict verdict_type>
void enable_get_problem_verdict<verdict_type>::set_problem_verdict(std::shared_ptr<verdict_type> verdict) {
    problem_verdict = verdict;
}

} // namespace oink_judge::services::test_node
