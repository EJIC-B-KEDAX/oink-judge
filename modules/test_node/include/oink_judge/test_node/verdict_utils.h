#pragma once
#include "oink_judge/test_node/verdict.hpp"

namespace oink_judge::test_node {

auto loadVerdictFromMeta(const std::string& test_name, const std::string& path_to_meta) -> std::shared_ptr<Verdict>;

auto loadVerdictFromCheckerOutput(const std::string& test_name, const std::string& path_to_meta,
                                  const std::string& path_to_checker_output) -> std::shared_ptr<Verdict>;

} // namespace oink_judge::test_node
