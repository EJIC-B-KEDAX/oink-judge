#pragma once

#include <string>
#include "ProblemTable.h"

namespace oink_judge::backend::management {

class Problem {
public:
    explicit Problem(const std::string &id);

    virtual ~Problem() = default;

    // getters

    std::string get_id() const;
    std::string get_short_name() const;
    double get_ok_score() const;
    const ProblemTable &get_table() const;
    const ProblemConfig &get_config() const;

    // setters

    void set_ok_score(double score);

    // another methods

    double get_participant_score(const std::string &username) const;

    virtual void handle_submission(const std::string &submission_id) = 0;
protected:
    ProblemTable &access_table();
    ProblemConfig &access_config();

private:
    std::string _id;

    double _ok_score = 100;

    ProblemConfig _config;
    ProblemTable _table;
};

} // namespace oink_judge::backend::management
