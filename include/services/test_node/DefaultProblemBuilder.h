#pragma once 

#include "ProblemBuilder.hpp"
#include "enable_get_test_by_name_impl.h"

namespace oink_judge::services::test_node {

class DefaultProblemBuilder : public ProblemBuilder, public enable_get_test_by_name_impl {
public:
    DefaultProblemBuilder(const std::string &problem_id);
    std::shared_ptr<Test> build() override;

    constexpr static auto REGISTERED_NAME = "DefaultProblemBuilder";

private:
    std::string _problem_id;
};

} // namespace oink_judge::services::test_node
