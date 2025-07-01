#pragma once

#include <string>
#include "ProblemConfig.h"

namespace oink_judge::backend::management {

using Subtask = testing::Subtask;
using Test = testing::Test;

class ProblemTable {
public:
    explicit ProblemTable(const ProblemConfig& problem_config);

    bool check_table_actuality();

    [[nodiscard]] std::string get_table_name() const;
    [[nodiscard]] std::vector<std::string> get_columns() const;

    void set_total_score(const std::string &username, double total_score);
    void set_tested_on_revision(const std::string &username, int revision);
    void set_score_on_testset(const std::string &username, const Testset &testset, double score);
    void set_score_on_subtask(const std::string &username, const Testset &testset, const Subtask &subtask, double score);
    void set_score_on_test(const std::string &username, const Testset &testset, const Subtask &subtask, const Test &test, double score);
    void add_new_user(const std::string &username);

    [[nodiscard]] double get_total_score(const std::string &username) const;
    [[nodiscard]] int get_tested_on_revision(const std::string &username) const;
    [[nodiscard]] double get_score_on_testset(const std::string &username, const Testset &testset) const;
    [[nodiscard]] double get_score_on_subtask(const std::string &username, const Testset &testset, const testing::Subtask &subtask) const;
    [[nodiscard]] double get_score_on_test(const std::string &username, const Testset &testset, const testing::Subtask &subtask, const Test &test) const;
    [[nodiscard]] bool does_user_have_submission(const std::string &username) const;

private:
    [[nodiscard]] std::vector<std::string> get_right_columns() const;
    void touch_table();

    std::string _id;
    std::string _short_name;

    const ProblemConfig &_config;
};

} // namespace oink_judge::backend::management
