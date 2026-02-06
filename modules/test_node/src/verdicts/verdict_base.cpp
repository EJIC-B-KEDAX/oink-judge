#include "oink_judge/test_node/verdicts/verdict_base.h"

namespace oink_judge::test_node {

VerdictBase::VerdictBase(std::string test_name)
    : test_name_(std::move(test_name)), info_{.type = VerdictType(VerdictType::WRONG_T, "Empty", "EM"),
                                              .score = 0.0,
                                              .time_used = 0.0,
                                              .memory_used = 0.0,
                                              .real_time_used = 0.0},
      additional_info_(json::object()) {}

auto VerdictBase::getType() const -> VerdictType { return info_.type; }

auto VerdictBase::getScore() const -> double { return info_.score; }

auto VerdictBase::toJson(int verbose) const -> json {
    json j;
    j["verdict"] = {
        {"full_name", info_.type.full_name},
        {"short_name", info_.type.short_name},
    };
    j["score"] = info_.score;
    if (verbose >= 1) {
        j["time_used"] = info_.time_used;
        j["memory_used"] = info_.memory_used;
        j["real_time_used"] = info_.real_time_used;
    }
    if (verbose >= 2) {
        j["additional_info"] = additional_info_;
    }
    return j;
}

auto VerdictBase::setInfo(const VerdictInfo& info) -> void { info_ = info; }

auto VerdictBase::addToAdditionalInfo(const std::string& key, const json& value) -> void { additional_info_[key] = value; }

auto VerdictBase::clearAdditionalInfo() -> void { additional_info_ = json::object(); }

auto VerdictBase::getInfo() const -> const VerdictBase::VerdictInfo& { return info_; }

auto VerdictBase::getAdditionalInfo() const -> const json& { return additional_info_; }

auto VerdictBase::getTestName() const -> const std::string& { return test_name_; }

} // namespace oink_judge::test_node
