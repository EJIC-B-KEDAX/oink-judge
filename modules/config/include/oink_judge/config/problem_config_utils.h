#pragma once
#include <optional>
#include <pugixml.hpp>
#include <string>
#include <vector>

namespace oink_judge::problem_config {

auto getFullProblemConfig(const std::string& problem_id) -> pugi::xml_document&;

auto getProblemConfig(const std::string& problem_id) -> std::optional<pugi::xml_node>;

auto getProblemBuilderName(const std::string& problem_id) -> std::optional<std::string>;

auto getTestConfig(const std::string& problem_id, const std::string& test_name) -> std::optional<pugi::xml_node>;

auto getAllTestNames(const std::string& problem_id) -> std::optional<std::vector<std::string>>;

} // namespace oink_judge::problem_config
