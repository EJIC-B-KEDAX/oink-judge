#pragma once

#include <string>
#include <vector>
#include "backend/testing/Testset.h"
#include <pugixml.hpp>


namespace oink_judge::backend::management {

using Testset = testing::Testset;
using Subtask = testing::Subtask;
using Test = testing::Test;

class ProblemConfig {
public:
    explicit ProblemConfig(const std::string &id);
    ~ProblemConfig();

    std::string get_id() const;
    std::string get_short_name() const;
    const std::vector<Testset*> &get_testsets() const;

private:
    std::string _id;

    pugi::xml_document _problem_xml;

    std::vector<Testset*> _testsets;
    std::vector<Subtask*> _subtasks;
    std::vector<Test*> _tests;
};

} // namespace oink_judge::backend::management