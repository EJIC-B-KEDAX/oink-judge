#pragma once
#include "parameterized_type_factory.hpp"

namespace oink_judge::factory {

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

template <smart_or_raw_pointer Base, typename... Args>
auto ParameterizedTypeFactory<Base, Args...>::parseName(const std::string& name) -> std::pair<std::string, std::string> {
    if (name.empty()) {
        return std::make_pair("", "");
    }

    if (name.back() != ')') {
        return std::make_pair(name, "");
    }

    int right = static_cast<int>(name.size()) - 1;
    int left = -1;

    int balance = 0;
    for (int i = static_cast<int>(name.size()) - 1; i >= 0; i--) {
        if (name[i] == '(') {
            balance--;
        } else if (name[i] == ')') {
            balance++;
        }

        if (balance == 0) {
            left = i;
            break;
        }
    }

    if (left == -1) {
        return std::make_pair(name, "");
    }

    return std::make_pair(name.substr(0, left), name.substr(left + 1, right - left - 1));
}

} // namespace oink_judge::factory
