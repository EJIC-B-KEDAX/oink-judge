#pragma once
#include "Verdict.hpp"

namespace oink_judge::services::test_node {

std::shared_ptr<Verdict> load_verdict_from_meta(const std::string &test_name, const std::string &path_to_meta);

std::shared_ptr<Verdict> load_verdict_from_checker_output(const std::string &test_name, const std::string &path_to_meta, const std::string &path_to_checker_output);

} // namespace oink_judge::services::test_node
