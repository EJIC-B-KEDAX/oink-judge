#include "services/test_node/verdicts/VerdictBase.h"

namespace oink_judge::services::test_node {

VerdictBase::VerdictBase(const std::string &test_name)
    : _test_name(test_name), _info{VerdictType(VerdictType::_WRONG, "Empty", "EM"), 0.0, 0.0, 0.0, 0.0}, _additional_info(json::object()) {}

VerdictType VerdictBase::get_type() const {
    return _info.type;
}

double VerdictBase::get_score() const {
    return _info.score;
}

json VerdictBase::to_json(int verbose) const {
    json j;
    j["verdict"] = {
        {"full_name", _info.type.full_name},
        {"short_name", _info.type.short_name},
    };
    j["score"] = _info.score;
    if (verbose >= 1) {
        j["time_used"] = _info.time_used;
        j["memory_used"] = _info.memory_used;
        j["real_time_used"] = _info.real_time_used;
    }
    if (verbose >= 2) {
        j["additional_info"] = _additional_info;
    }
    return j;
}

void VerdictBase::set_info(const VerdictInfo& info) {
    _info = info;
}

void VerdictBase::add_to_additional_info(const std::string& key, const json& value) {
    _additional_info[key] = value;
}

void VerdictBase::clear_additional_info() {
    _additional_info = json::object();
}

const VerdictBase::VerdictInfo& VerdictBase::get_info() const {
    return _info;
}

const json& VerdictBase::get_additional_info() const {
    return _additional_info;
}

const std::string& VerdictBase::get_test_name() const {
    return _test_name;
}

} // namespace oink_judge::services::test_node
