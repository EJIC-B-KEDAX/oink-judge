#pragma once
#include "oink_judge/test_node/verdict_type.hpp"

#include <nlohmann/json.hpp>

namespace oink_judge::test_node {

using json = nlohmann::json;

class Verdict {
  public:
    Verdict(const Verdict&) = delete;
    auto operator=(const Verdict&) -> Verdict& = delete;
    Verdict(Verdict&&) = delete;
    auto operator=(Verdict&&) -> Verdict& = delete;
    virtual ~Verdict() = default;

    [[nodiscard]] virtual auto getType() const -> VerdictType = 0;
    [[nodiscard]] virtual auto getScore() const -> double = 0;
    [[nodiscard]] virtual auto toJson(int verbose) const -> json = 0;

  protected:
    Verdict() = default;
};

} // namespace oink_judge::test_node
