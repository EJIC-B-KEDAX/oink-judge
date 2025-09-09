#include "services/test_node/DefaultVerdict.h"

namespace oink_judge::services::test_node {

DefaultVerdict::DefaultVerdict(const std::string &test_name)
    : _test_name(test_name), _info{VerdictType(VerdictType::WRONG, "Empty", "EM"), 0.0, 0.0, 0.0, 0.0}, _additional_info(json::object()) {}

VerdictType DefaultVerdict::get_type() const {
    return _info.type;
}

double DefaultVerdict::get_score() const {
    return _info.score;
}

json DefaultVerdict::to_json(int verbose) const {
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

void DefaultVerdict::set_info(const VerdictInfo& info) {
    _info = info;
}

void DefaultVerdict::add_to_additional_info(const std::string& key, const json& value) {
    _additional_info[key] = value;
}

void DefaultVerdict::clear_additional_info() {
    _additional_info = json::object();
}

const DefaultVerdict::VerdictInfo& DefaultVerdict::get_info() const {
    return _info;
}

const json& DefaultVerdict::get_additional_info() const {
    return _additional_info;
}

const std::string& DefaultVerdict::get_test_name() const {
    return _test_name;
}

} // namespace oink_judge::services::test_node
