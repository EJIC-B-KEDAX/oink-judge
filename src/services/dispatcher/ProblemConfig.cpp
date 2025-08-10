#include "services/dispatcher/ProblemConfig.h"
#include <format>
#include "services/test_node/OutputTest.h"
#include "services/test_node/SimpleSubtask.h"
#include "services/test_node/SimpleTestset.h"
#include "config/Config.h"
#include <iostream>

namespace oink_judge::services::dispatcher {

using Config = config::Config;
using SimpleTestset = test_node::SimpleTestset;
using SimpleSubtask = test_node::SimpleSubtask;
using OutputTest = test_node::OutputTest;

ProblemConfig::ProblemConfig(const std::string &id) : _id(id) {
    std::string path_to_problems = Config::config()["directories"]["problems"];
    std::string path_to_config = path_to_problems + "/" + _id + "/problem.xml";
    if (!_problem_xml.load_file(path_to_config.c_str())) {
        throw std::runtime_error("Failed to load problem xml file " + path_to_config);
    }

    pugi::xml_node judge_node = _problem_xml.child("problem").child("judging");

    for (pugi::xml_node testset : judge_node.children("testset")) {

        std::string testset_id = testset.attribute("name").as_string();

        double time_limit = testset.child("time-limit").text().as_double() / 1000;
        double memory_limit = testset.child("memory-limit").text().as_double() / 1024;
        double idle_limit = std::max(2 * time_limit, 5.0);

        Testset *ts = new SimpleTestset(testset_id, time_limit, memory_limit, idle_limit);
        _testsets.push_back(ts);

        if (pugi::xml_node groups = testset.child("groups")) {
        } else {
            Subtask *st = new SimpleSubtask("main_group", 100);
            _subtasks.push_back(st);
            ts->push_subtask(st);

            int ptr = 1;
            for (pugi::xml_node test : testset.child("tests").children("test")) {
                Test *tst = new OutputTest(std::to_string(ptr), path_to_problems + "/" + _id + "/" + testset_id + "/" + std::format("{:02}", ptr),
                    path_to_problems + "/" + _id + "/" + testset_id + "/" + std::format("{:02}", ptr) + ".a");
                _tests.push_back(tst);
                st->push_test(tst);
                ptr++;
            }
        }
    }
}

ProblemConfig::~ProblemConfig() {
    for (int i = 0; i < _testsets.size(); i++) {
        delete _testsets[i];
    }
    for (int i = 0; i < _subtasks.size(); i++) {
        delete _subtasks[i];
    }
    for (int i = 0; i < _tests.size(); i++) {
        delete _tests[i];
    }
}


std::string ProblemConfig::get_id() const {
    return _id;
}

std::string ProblemConfig::get_short_name() const {
    return _problem_xml.child("problem").attribute("short-name").as_string();
}

const std::vector<Testset*> &ProblemConfig::get_testsets() const {
    return _testsets;
}


} // namespace oink_judge::services::dispatcher
