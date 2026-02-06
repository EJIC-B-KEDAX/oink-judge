#include "oink_judge/test_node/verdict_builders/verdict_builder_sum.h"

namespace oink_judge::test_node {

namespace {

[[maybe_unused]] const bool REGISTERED = []() -> bool {
    VerdictBuilderFactory::instance().registerType(
        VerdictBuilderSum::REGISTERED_NAME,
        [](const std::string& params, const std::string& test_name) -> std::unique_ptr<VerdictBuilderSum> {
            return std::make_unique<VerdictBuilderSum>(test_name);
        });

    return true;
}();

} // namespace

VerdictBuilderSum::VerdictBuilderSum(std::string test_name)
    : test_name_(std::move(test_name)), current_verdict_(std::make_shared<DefaultVerdict>(test_name_)) {
    clear();
}

auto VerdictBuilderSum::clear() -> void {
    current_verdict_ = std::make_shared<DefaultVerdict>(test_name_);
    current_verdict_->setInfo({.type = VerdictType::ACCEPTED, .score = 0, .time_used = 0, .memory_used = 0, .real_time_used = 0});
}

auto VerdictBuilderSum::addVerdict(std::shared_ptr<DefaultVerdict> verdict) -> void {
    current_verdict_->addToAdditionalInfo(verdict->getTestName(), verdict->toJson(2));
    auto info = verdict->getInfo();
    auto current_info = current_verdict_->getInfo();
    DefaultVerdict::VerdictInfo new_info;
    VerdictType new_type;
    if (current_info.type.type == VerdictType::Type::ACCEPTED_T) {
        if (info.type.type == VerdictType::Type::WRONG_T || info.type.type == VerdictType::Type::SKIPPED_T) {
            new_type = VerdictType::PARTIAL;
        } else {
            new_type = info.type;
        }
    } else if (current_info.type.type == VerdictType::Type::PARTIAL_T) {
        if (info.type.type == VerdictType::Type::FAILED_T) {
            new_type = info.type;
        } else {
            new_type = current_info.type;
        }
    } else if (current_info.type.type == VerdictType::Type::SKIPPED_T) {
        if (info.type.type == VerdictType::Type::SKIPPED_T) {
            new_type = current_info.type;
        } else {
            new_type = info.type;
        }
    } else if (current_info.type.type == VerdictType::Type::WRONG_T) {
        if (info.type.type == VerdictType::Type::FAILED_T || info.type.type == VerdictType::Type::PARTIAL_T) {
            new_type = info.type;
        } else if (info.type.type == VerdictType::Type::ACCEPTED_T) {
            new_type = VerdictType::PARTIAL;
        } else {
            new_type = current_info.type;
        }
    } else { // current_info.type == FAILED
        new_type = current_info.type;
    }
    new_info.type = new_type;
    new_info.score = current_info.score + info.score;
    new_info.time_used = std::max(current_info.time_used, info.time_used);
    new_info.memory_used = std::max(current_info.memory_used, info.memory_used);
    new_info.real_time_used = std::max(current_info.real_time_used, info.real_time_used);
    current_verdict_->setInfo(new_info);
}

auto VerdictBuilderSum::canScoreChange() const -> bool { return current_verdict_->getInfo().score != 0.0; }

auto VerdictBuilderSum::finalize() -> std::shared_ptr<DefaultVerdict> { return current_verdict_; }

} // namespace oink_judge::test_node
