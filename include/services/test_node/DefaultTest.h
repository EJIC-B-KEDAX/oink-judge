#pragma once

#include "services/test_node/Test.hpp"

namespace oink_judge::services::test_node {

class DefaultTest : public Test {
public:
    DefaultTest(const std::string &problem_id, const std::string &name);

    std::shared_ptr<Verdict> run(const std::string &submission_id, const std::vector<std::string> &boxes, json additional_params) override;
    std::shared_ptr<Verdict> skip(const std::string &submission_id) override;
    size_t boxes_required() const override;
    const std::string &get_name() const override;

    constexpr static auto REGISTERED_NAME = "default";

private:
    std::string _name;
    std::string _input_path;
    std::string _answer_path;
};

} // namespace oink_judge::services::test_node
