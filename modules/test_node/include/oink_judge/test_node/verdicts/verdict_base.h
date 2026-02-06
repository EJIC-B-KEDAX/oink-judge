#pragma once
#include "oink_judge/test_node/verdict.hpp"

namespace oink_judge::test_node {

class VerdictBase : public Verdict {
  public:
    struct VerdictInfo {
        VerdictType type;
        double score{};
        double time_used{};
        double memory_used{};
        double real_time_used{};
    };

    explicit VerdictBase(std::string test_name);

    [[nodiscard]] auto getType() const -> VerdictType override;
    [[nodiscard]] auto getScore() const -> double override;
    [[nodiscard]] auto toJson(int verbose) const -> json override;

    auto setInfo(const VerdictInfo& info) -> void;
    auto addToAdditionalInfo(const std::string& key, const json& value) -> void;
    auto clearAdditionalInfo() -> void;

    [[nodiscard]] auto getInfo() const -> const VerdictInfo&;
    [[nodiscard]] auto getAdditionalInfo() const -> const json&;
    [[nodiscard]] auto getTestName() const -> const std::string&;

  private:
    VerdictInfo info_;
    json additional_info_;
    std::string test_name_;
};

} // namespace oink_judge::test_node
