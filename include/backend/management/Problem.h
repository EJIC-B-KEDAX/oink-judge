#pragma once

#include <string>

namespace oink_judge::backend::management {

class Problem {
public:
    explicit Problem(std::string id);

    virtual ~Problem() = default;

    // getters

    const std::string &get_id() const;

    const std::string &get_short_name() const;

    double get_ok_score() const;

    // setters

    void set_ok_score(double score);

    // another methods

    double get_participant_score(std::string participant_id) const;

    double get_participant_score(std::string participant_id, std::string test_id) const;

    virtual void handle_submission(std::string submission_id) = 0;

private:
    std::string _id;

    double _ok_score = 100;
};

} // namespace oink_judge::backend::management
