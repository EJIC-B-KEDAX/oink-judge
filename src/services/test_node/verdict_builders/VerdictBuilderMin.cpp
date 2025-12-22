#include "services/test_node/verdict_builders/VerdictBuilderMin.h"

namespace oink_judge::services::test_node {

namespace {

[[maybe_unused]] bool registered = []() {
    VerdictBuilderFactory::instance().register_type(
        VerdictBuilderMin::REGISTERED_NAME,
        [](const std::string &params, const std::string &test_name) -> std::unique_ptr<VerdictBuilderMin> {
            return std::make_unique<VerdictBuilderMin>(test_name);
        }
    );

    return true;
}();

} // namespace

VerdictBuilderMin::VerdictBuilderMin(const std::string &test_name) : _test_name(test_name),
    _current_verdict(std::make_shared<DefaultVerdict>(test_name)) {
        clear();
    }

void VerdictBuilderMin::clear() {
    _current_verdict = std::make_shared<DefaultVerdict>(_test_name);
    _current_verdict->set_info({VerdictType::ACCEPTED, 100, 0, 0, 0});
}

void VerdictBuilderMin::add_verdict(std::shared_ptr<DefaultVerdict> verdict) {
    _current_verdict->add_to_additional_info(verdict->get_test_name(), verdict->to_json(2));
    auto info = verdict->get_info();
    auto current_info = _current_verdict->get_info();
    DefaultVerdict::VerdictInfo new_info;
    VerdictType new_type;
    if (current_info.type.type == VerdictType::Type::_ACCEPTED) {
        new_type = info.type;
    } else if (current_info.type.type == VerdictType::Type::_PARTIAL) {
        if (info.type.type == VerdictType::Type::_ACCEPTED) {
            new_type = current_info.type;
        } else {
            new_type = info.type;
        }
    } else if (current_info.type.type == VerdictType::Type::_SKIPPED) {
        if (info.type.type == VerdictType::Type::_FAILED || info.type.type == VerdictType::Type::_WRONG) {
            new_type = info.type;
        } else if (info.type.type == VerdictType::Type::_PARTIAL) {
            new_type = info.type;
        } else {
            new_type = current_info.type;
        }
    } else if (current_info.type.type == VerdictType::Type::_WRONG) {
        if (info.type.type == VerdictType::Type::_FAILED) {
            new_type = info.type;
        } else {
            new_type = current_info.type;
        }
    } else { // current_info.type == FAILED
        new_type = current_info.type;
    }
    new_info.type = new_type;
    new_info.score = std::min(current_info.score, info.score);
    new_info.time_used = std::max(current_info.time_used, info.time_used);
    new_info.memory_used = std::max(current_info.memory_used, info.memory_used);
    new_info.real_time_used = std::max(current_info.real_time_used, info.real_time_used);
    _current_verdict->set_info(new_info);
}

bool VerdictBuilderMin::can_score_change() const {
    return _current_verdict->get_info().score != 0.0;
}

std::shared_ptr<DefaultVerdict> VerdictBuilderMin::finalize() {
    return _current_verdict;
}

} // namespace oink_judge::services::test_node
