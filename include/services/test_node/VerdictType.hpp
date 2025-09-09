#pragma once

#include <string>

namespace oink_judge::services::test_node {

struct VerdictType {
public:
    enum Type {
        ACCEPTED,
        PARTIAL,
        SKIPPED,
        WRONG,
        FAILED
    };
    Type type;
    std::string full_name;
    std::string short_name;
};

} // namespace oink_judge::services::test_node
