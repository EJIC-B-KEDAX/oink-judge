#pragma once
#include "oink_judge/test_node/verdict_builder.hpp"

namespace oink_judge::test_node {

class VerdictBuilderSum : public VerdictBuilder {
  public:
    VerdictBuilderSum(std::string test_name);

    auto clear() -> void override;
    auto addVerdict(std::shared_ptr<DefaultVerdict> verdict) -> void override;
    [[nodiscard]] auto canScoreChange() const -> bool override;
    [[nodiscard]] auto finalize() -> std::shared_ptr<DefaultVerdict> override;

    constexpr static auto REGISTERED_NAME = "sum";

  private:
    std::string test_name_;
    std::shared_ptr<DefaultVerdict> current_verdict_;
};

} // namespace oink_judge::test_node
