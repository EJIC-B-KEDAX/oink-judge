#pragma once
#include "parameterized_type_factory.hpp"

#include <string>

namespace oink_judge::factory {

namespace {

auto findFirstSymbol(const std::string::const_iterator& begin, const std::string::const_iterator& end, const std::string& symbols)
    -> std::string::const_iterator {
    int balance = 0;
    for (auto it = begin; it != end; ++it) {
        if (*it == '\\') {
            // Skip escaped characters
            ++it;
            if (it == end) {
                break;
            }
            continue;
        }

        if (*it == ')') {
            --balance;
        }

        if (symbols.find(*it) != std::string::npos && balance == 0) {
            return it;
        }

        if (*it == '(') {
            ++balance;
        }
    }
    return end;
}

} // namespace

template <smart_or_raw_pointer Base, typename... Args>
auto ParameterizedTypeFactory<Base, Args...>::instance() -> ParameterizedTypeFactory<Base, Args...>& {
    static ParameterizedTypeFactory fabric;
    return fabric;
}

template <smart_or_raw_pointer Base, typename... Args>
auto ParameterizedTypeFactory<Base, Args...>::create(const std::string& name, Args&&... args) const -> Base {
    auto [real_name, params] = parseName(name);

    auto iter = this->getRegisteredTypes().find(real_name);
    if (iter == this->getRegisteredTypes().end()) {
        throw std::runtime_error("Unknown type: " + real_name);
    }

    return iter->second(params, std::forward<Args>(args)...);
}

inline auto parseName(const std::string& name) -> std::pair<std::string, std::string> {
    std::string normalized_name = normalizeArgument(name, true);
    if (normalized_name.empty()) {
        return std::make_pair("", "");
    }

    int left = static_cast<int>(findFirstSymbol(normalized_name.begin(), normalized_name.end(), "(") - normalized_name.begin());
    int right = static_cast<int>(findFirstSymbol(normalized_name.begin(), normalized_name.end(), ")") - normalized_name.begin());

    if (left == right && left == static_cast<int>(normalized_name.size())) {
        return std::make_pair(normalized_name, "");
    }

    if (left == static_cast<int>(normalized_name.size()) || right != static_cast<int>(normalized_name.size()) - 1 ||
        left > right) {
        throw std::runtime_error("Invalid type name: " + normalized_name);
    }

    return std::make_pair(normalized_name.substr(0, left), normalized_name.substr(left + 1, right - left - 1));
}

inline auto parseParameters(const std::string& params, const std::string& delimiters) -> std::vector<std::string> {
    std::vector<std::string> result;
    auto begin = params.begin();
    auto end = params.end();

    while (begin != end) {
        auto symbol_it = findFirstSymbol(begin, end, delimiters);
        if (symbol_it == end) {
            result.emplace_back(begin, end);
            break;
        }
        result.emplace_back(begin, symbol_it);
        begin = symbol_it + 1;
    }

    return result;
}

inline auto normalizeArgument(const std::string& name, bool remove_whitespaces) -> std::string {
    std::string result;
    int balance = 0;
    for (size_t i = 0; i < name.size(); ++i) {
        char c = name[i];
        if (c == '\\' && balance == 0) {
            // Preserve escaped characters as-is
            result += c;
            if (i + 1 < name.size()) {
                result += name[i + 1];
                ++i;
            }
            continue;
        }

        if (c == '(') {
            ++balance;
        } else if (c == ')') {
            --balance;
        }

        // Remove whitespace outside of parentheses
        if (((std::isspace(static_cast<unsigned char>(c)) == 0) || balance > 0) && remove_whitespaces) {
            result += c;
        }
    }
    return result;
}

} // namespace oink_judge::factory
