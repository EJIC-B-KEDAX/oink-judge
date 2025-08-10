#pragma once

#include "services/test_node/Test.h"

namespace oink_judge::services::test_node {

class OutputTest : public Test {
public:
    OutputTest(const std::string &test_id, const std::string &input_path, const std::string &answer_path);

    std::string get_input_path() const;
    std::string get_answer_path() const;

    Verdict run(const Testset &testset, const Subtask &subtask, const std::string &box_id_to_run, const std::string &box_id_to_check) const override;
private:
    std::string _input_path;
    std::string _answer_path;
};

} // namespace oink_judge::services::test_node
