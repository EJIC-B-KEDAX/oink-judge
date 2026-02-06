#pragma once
#include "oink_judge/test_node/verdicts/default_verdict.h"

#include <oink_judge/factory/parameterized_type_factory.hpp>

namespace oink_judge::test_node {

class VerdictBuilder {
  public:
    VerdictBuilder(const VerdictBuilder&) = delete;
    auto operator=(const VerdictBuilder&) -> VerdictBuilder& = delete;
    VerdictBuilder(VerdictBuilder&&) = delete;
    auto operator=(VerdictBuilder&&) -> VerdictBuilder& = delete;
    virtual ~VerdictBuilder() = default;

    virtual auto clear() -> void = 0;
    virtual auto addVerdict(std::shared_ptr<DefaultVerdict> verdict) -> void = 0;
    [[nodiscard]] virtual auto canScoreChange() const -> bool = 0;
    [[nodiscard]] virtual auto finalize() -> std::shared_ptr<DefaultVerdict> = 0;

  protected:
    VerdictBuilder() = default;
};

using VerdictBuilderFactory = factory::ParameterizedTypeFactory<std::unique_ptr<VerdictBuilder>, const std::string&>;

} // namespace oink_judge::test_node
