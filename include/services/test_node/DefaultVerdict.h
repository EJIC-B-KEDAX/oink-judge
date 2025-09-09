#pragma once

#include "Verdict.hpp"

namespace oink_judge::services::test_node {

class DefaultVerdict : public Verdict {
public:
    struct VerdictInfo {
        VerdictType type;
        double score;
        double time_used;
        double memory_used;
        double real_time_used;
    };

    explicit DefaultVerdict(const std::string &test_name);

    VerdictType get_type() const override;
    double get_score() const override;
    json to_json(int verbose) const override;

    void set_info(const VerdictInfo& info);
    void add_to_additional_info(const std::string& key, const json& value);
    void clear_additional_info();

    const VerdictInfo& get_info() const;
    const json& get_additional_info() const;
    const std::string& get_test_name() const;

private:
    VerdictInfo _info;
    json _additional_info;
    std::string _test_name;
};

} // namespace oink_judge::services::test_node
