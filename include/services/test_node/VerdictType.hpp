#pragma once
#include <string>

namespace oink_judge::services::test_node {

struct VerdictType {
public:
    enum Type {
        _ACCEPTED,
        _PARTIAL,
        _SKIPPED,
        _WRONG,
        _FAILED
    };
    Type type;
    std::string full_name;
    std::string short_name;

    static const VerdictType ACCEPTED;
    static const VerdictType OK;
    static const VerdictType WRONG_ANSWER;
    static const VerdictType TIME_LIMIT_EXCEEDED;
    static const VerdictType MEMORY_LIMIT_EXCEEDED;
    static const VerdictType RUNTIME_ERROR;
    static const VerdictType PRESENTATION_ERROR;
    static const VerdictType COMPILATION_ERROR;
    static const VerdictType IDLE_LIMIT_EXCEEDED;
    static const VerdictType PARTIAL;
    static const VerdictType SKIPPED;
    static const VerdictType SECURITY_VIOLATION;
    static const VerdictType FAILED;
};

inline const VerdictType VerdictType::ACCEPTED = {
    VerdictType::Type::_ACCEPTED,
    "Accepted",
    "AC"
};

inline const VerdictType VerdictType::OK = {
    VerdictType::Type::_ACCEPTED,
    "OK",
    "OK"
};

inline const VerdictType VerdictType::WRONG_ANSWER = {
    VerdictType::Type::_WRONG,
    "Wrong Answer",
    "WA"
};

inline const VerdictType VerdictType::TIME_LIMIT_EXCEEDED = {
    VerdictType::Type::_WRONG,
    "Time Limit Exceeded",
    "TL"
};

inline const VerdictType VerdictType::MEMORY_LIMIT_EXCEEDED = {
    VerdictType::Type::_WRONG,
    "Memory Limit Exceeded",
    "ML"
};

inline const VerdictType VerdictType::RUNTIME_ERROR = {
    VerdictType::Type::_WRONG,
    "Runtime Error",
    "RE"
};

inline const VerdictType VerdictType::PRESENTATION_ERROR = {
    VerdictType::Type::_WRONG,
    "Presentation Error",
    "PE"
};

inline const VerdictType VerdictType::COMPILATION_ERROR = {
    VerdictType::Type::_WRONG,
    "Compilation Error",
    "CE"
};

inline const VerdictType VerdictType::IDLE_LIMIT_EXCEEDED = {
    VerdictType::Type::_WRONG,
    "Idle Limit Exceeded",
    "IL"
};

inline const VerdictType VerdictType::PARTIAL = {
    VerdictType::Type::_PARTIAL,
    "Partial",
    "PT"
};

inline const VerdictType VerdictType::SKIPPED = {
    VerdictType::Type::_SKIPPED,
    "Skipped",
    "SK"
};

inline const VerdictType VerdictType::SECURITY_VIOLATION = {
    VerdictType::Type::_WRONG,
    "Security Violation",
    "SV"
};

inline const VerdictType VerdictType::FAILED = {
    VerdictType::Type::_FAILED,
    "Failed",
    "FAIL"
};

} // namespace oink_judge::services::test_node
