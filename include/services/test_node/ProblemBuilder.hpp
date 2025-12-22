#pragma once 
#include "Test.hpp"
#include "TypeFactory.hpp"

namespace oink_judge::services::test_node {

class ProblemBuilder {
public:
    virtual ~ProblemBuilder() = default;
    virtual std::shared_ptr<Test> build() = 0;
};

using ProblemBuilderFactory = TypeFactory<std::shared_ptr<ProblemBuilder>, const std::string&>;

} // namespace oink_judge::services::test_node
