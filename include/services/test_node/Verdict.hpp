#pragma once

#include <string>
#include <nlohmann/json.hpp>
#include "VerdictType.hpp"

namespace oink_judge::services::test_node {

using json = nlohmann::json;

class Verdict {
public:
    virtual ~Verdict() = default;
    virtual VerdictType get_type() const = 0;
    virtual double get_score() const = 0;
    virtual json to_json(int verbose) const = 0;
};

} // namespace oink_judge::services::test_node
