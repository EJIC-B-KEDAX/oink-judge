#include "services/test_node/DefaultVerdictBuilder.h"

namespace oink_judge::services::test_node {

namespace {

[[maybe_unused]] bool registered = []() {
    VerdictBuilderFactory::instance().register_type(
        DefaultVerdictBuilder::REGISTERED_NAME,
        [](const std::string &params, const std::string &test_name) -> std::unique_ptr<VerdictBuilder> {
            return std::make_unique<DefaultVerdictBuilder>(test_name);
        }
    );

    return true;
}();

} // namespace

DefaultVerdictBuilder::DefaultVerdictBuilder(const std::string &test_name) : _test_name(test_name),
    _current_verdict(std::make_shared<DefaultVerdict>(test_name)) {
        clear();
    }

void DefaultVerdictBuilder::clear() {
    _current_verdict = std::make_shared<DefaultVerdict>(_test_name);
    _current_verdict->set_info({VerdictType(VerdictType::ACCEPTED, "AC", "Accepted"), 100, 0, 0, 0});
}

void DefaultVerdictBuilder::add_verdict(std::shared_ptr<DefaultVerdict> verdict) {
    _current_verdict->add_to_additional_info(verdict->get_test_name(), verdict->to_json(2));
    auto info = verdict->get_info();
    auto current_info = _current_verdict->get_info();
    DefaultVerdict::VerdictInfo new_info;
    VerdictType new_type;
    if (current_info.type.type == VerdictType::ACCEPTED) {
        new_type = info.type;
    } else if (current_info.type.type == VerdictType::PARTIAL) {
        if (info.type.type == VerdictType::ACCEPTED) {
            new_type = current_info.type;
        } else {
            new_type = info.type;
        }
    } else if (current_info.type.type == VerdictType::SKIPPED) {
        if (info.type.type == VerdictType::FAILED || info.type.type == VerdictType::WRONG) {
            new_type = info.type;
        } else if (info.type.type == VerdictType::PARTIAL) {
            new_type = info.type;
        } else {
            new_type = current_info.type;
        }
    } else if (current_info.type.type == VerdictType::WRONG) {
        if (info.type.type == VerdictType::FAILED) {
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

bool DefaultVerdictBuilder::can_score_change() const {
    return _current_verdict->get_info().score != 0.0;
}

std::shared_ptr<DefaultVerdict> DefaultVerdictBuilder::finalize() {
    return _current_verdict;
}

} // namespace oink_judge::services::test_node
