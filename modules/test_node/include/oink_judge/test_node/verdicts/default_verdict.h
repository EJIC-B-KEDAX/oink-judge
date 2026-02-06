#pragma once
#include "oink_judge/test_node/verdicts/verdict_base.h"

namespace oink_judge::test_node {

class DefaultVerdict : public VerdictBase {
  public:
    explicit DefaultVerdict(std::string test_name);
};

} // namespace oink_judge::test_node