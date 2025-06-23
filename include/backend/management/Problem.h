#pragma once

#include <string>

namespace oink_judge::backend::management {

class Problem {
public:

    Problem(std::string id);

    virtual ~Problem() = default;

    std::string get_id() const;

    virtual double get_participant_score(std::string participant_id) const = 0;

    virtual double get_participant_score(std::string participant_id, std::string test_id) const = 0;

    virtual void handle_submission(std::string submission_id) = 0;

private:
    std::string _id;
};

} // namespace oink_judge::backend::management
