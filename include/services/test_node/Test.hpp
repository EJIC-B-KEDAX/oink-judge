#pragma once
#include <string>
#include <vector>
#include "Verdict.hpp"
#include "TypeFactory.hpp"

namespace oink_judge::services::test_node {

class ProblemBuilder;

class Test {
public:
    virtual ~Test() = default;
    virtual std::shared_ptr<Verdict> run(const std::string &submission_id, const std::vector<std::string> &boxes, json additional_params) = 0;
    virtual std::shared_ptr<Verdict> skip(const std::string &submission_id) = 0;
    virtual size_t boxes_required() const = 0;
    virtual const std::string &get_name() const = 0;
};

using TestFactory = TypeFactory<std::shared_ptr<Test>, ProblemBuilder*, const std::string&, const std::string&>;

} // namespace oink_judge::services::test_node
