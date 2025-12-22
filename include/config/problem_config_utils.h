#pragma once
#include <string>
#include <vector>
#include <pugixml.hpp>

namespace oink_judge::problem_config {

pugi::xml_node get_problem_config(const std::string &problem_id);

std::string get_problem_builder_name(const std::string &problem_id);

pugi::xml_node get_test_config(const std::string &problem_id, const std::string &test_name);

std::vector<std::string> get_all_test_names(const std::string &problem_id);

} // namespace oink_judge::problem_config
