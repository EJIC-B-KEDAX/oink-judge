#pragma once
#include <pugixml.hpp>

#include <filesystem>
#include <optional>
#include <string>
#include <vector>

namespace oink_judge::problem_config {

namespace fs = std::filesystem;

auto getFullProblemConfig(const std::string& problem_id) -> const pugi::xml_document&;

auto getProblemConfig(const std::string& problem_id) -> std::optional<pugi::xml_node>;

auto getProblemBuilderName(const std::string& problem_id) -> std::optional<std::string>;

auto getTestConfig(const std::string& problem_id, const std::string& test_name) -> std::optional<pugi::xml_node>;

auto getAllTestNames(const std::string& problem_id) -> std::optional<std::vector<std::string>>;

auto getPathToProblemStatements(const std::string& problem_id, const std::string& language, const std::string& type)
    -> std::optional<fs::path>;

auto getProblemStatements(const std::string& problem_id, const std::string& language, const std::string& type)
    -> std::optional<std::string>;

} // namespace oink_judge::problem_config
