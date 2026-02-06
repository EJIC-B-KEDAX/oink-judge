#pragma once
#include <cstdint>
#include <string>

namespace oink_judge::test_node {

struct VerdictType {
  public:
    enum Type : uint8_t { ACCEPTED_T, PARTIAL_T, SKIPPED_T, WRONG_T, FAILED_T };
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
    .type = VerdictType::Type::ACCEPTED_T, .full_name = "Accepted", .short_name = "AC"};

inline const VerdictType VerdictType::OK = {.type = VerdictType::Type::ACCEPTED_T, .full_name = "OK", .short_name = "OK"};

inline const VerdictType VerdictType::WRONG_ANSWER = {
    .type = VerdictType::Type::WRONG_T, .full_name = "Wrong Answer", .short_name = "WA"};

inline const VerdictType VerdictType::TIME_LIMIT_EXCEEDED = {
    .type = VerdictType::Type::WRONG_T, .full_name = "Time Limit Exceeded", .short_name = "TL"};

inline const VerdictType VerdictType::MEMORY_LIMIT_EXCEEDED = {
    .type = VerdictType::Type::WRONG_T, .full_name = "Memory Limit Exceeded", .short_name = "ML"};

inline const VerdictType VerdictType::RUNTIME_ERROR = {
    .type = VerdictType::Type::WRONG_T, .full_name = "Runtime Error", .short_name = "RE"};

inline const VerdictType VerdictType::PRESENTATION_ERROR = {
    .type = VerdictType::Type::WRONG_T, .full_name = "Presentation Error", .short_name = "PE"};

inline const VerdictType VerdictType::COMPILATION_ERROR = {
    .type = VerdictType::Type::WRONG_T, .full_name = "Compilation Error", .short_name = "CE"};

inline const VerdictType VerdictType::IDLE_LIMIT_EXCEEDED = {
    .type = VerdictType::Type::WRONG_T, .full_name = "Idle Limit Exceeded", .short_name = "IL"};

inline const VerdictType VerdictType::PARTIAL = {
    .type = VerdictType::Type::PARTIAL_T, .full_name = "Partial", .short_name = "PT"};

inline const VerdictType VerdictType::SKIPPED = {
    .type = VerdictType::Type::SKIPPED_T, .full_name = "Skipped", .short_name = "SK"};

inline const VerdictType VerdictType::SECURITY_VIOLATION = {
    .type = VerdictType::Type::WRONG_T, .full_name = "Security Violation", .short_name = "SV"};

inline const VerdictType VerdictType::FAILED = {.type = VerdictType::Type::FAILED_T, .full_name = "Failed", .short_name = "FAIL"};

} // namespace oink_judge::test_node
