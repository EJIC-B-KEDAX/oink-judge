#pragma once

#include <string>

namespace oink_judge::services::test_node {

class Verdict {
public:
    enum Type {
        OK,
        WA,
        PE,
        FAIL,
        TL,
        ML,
        IL,
        PS,
        RE,
        CE,
        SK,
        SV
    };

    enum ScoreMergePolicy {
        Sum,
        Minimum
    };

    Verdict() = default;

    void set_score(double score);
    void set_time_used(double time_used);
    void set_memory_used(double memory_used);
    void set_type(Type type);
    void set_score_merge_policy(ScoreMergePolicy policy);

    double get_score() const;
    double get_time_used() const;
    double get_memory_used() const;
    Type get_type() const;
    ScoreMergePolicy get_score_merge_policy() const;

private:
    double _score;

    double _time_used;
    double _memory_used;

    Type _type;
    ScoreMergePolicy _score_merge_policy;
};

Verdict load_verdict_from_meta(const std::string &path_to_meta);
Verdict load_verdict_from_checker_output(const std::string &path_to_meta, const std::string &path_to_checker_output);

Verdict operator+(const Verdict &verdict1, const Verdict &verdict2);

} // namespace oink_judge::services::test_node
