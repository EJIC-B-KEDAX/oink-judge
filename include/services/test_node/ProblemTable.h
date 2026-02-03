#pragma once
#include "services/test_node/Test.hpp"

#include <string>

namespace oink_judge::services::test_node {

class ProblemTable {
  public:
    explicit ProblemTable(const std::string& problem_id);

    bool check_table_actuality();

    [[nodiscard]] std::string get_table_name() const;
    [[nodiscard]] std::string get_raw_table_name() const;
    [[nodiscard]] std::vector<std::string> get_columns() const;

    void set_total_score(const std::string& username, double total_score);
    void set_tested_on_revision(const std::string& username, int revision);
    void set_score_on_test(const std::string& username, std::shared_ptr<Test> test, double score);
    void add_new_user(const std::string& username);

    [[nodiscard]] double get_total_score(const std::string& username) const;
    [[nodiscard]] int get_tested_on_revision(const std::string& username) const;
    [[nodiscard]] double get_score_on_test(const std::string& username, std::shared_ptr<Test> test) const;
    [[nodiscard]] bool does_user_have_submission(const std::string& username) const;

  private:
    [[nodiscard]] std::vector<std::string> get_right_columns() const;
    void touch_table();

    std::string id_;
};

} // namespace oink_judge::services::test_node
