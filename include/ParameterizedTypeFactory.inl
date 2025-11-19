#pragma once
#include "ParameterizedTypeFactory.hpp"

namespace oink_judge {

template<smart_or_raw_pointer Base, typename... Args>
ParameterizedTypeFactory<Base, Args...> &ParameterizedTypeFactory<Base, Args...>::instance() {
    static ParameterizedTypeFactory fabric;
    return fabric;
}

template<smart_or_raw_pointer Base, typename... Args>
Base ParameterizedTypeFactory<Base, Args...>::create(const std::string &name, Args &&... args) const {
    auto [real_name, params] = _parse_name(name);

    auto it = this->get_registered_types().find(real_name);
    if (it == this->get_registered_types().end()) {
        throw std::runtime_error("Unknown type: " + real_name);
    }

    return it->second(params, std::forward<Args>(args)...);
}

template<smart_or_raw_pointer Base, typename... Args>
std::pair<std::string, std::string> ParameterizedTypeFactory<Base, Args...>::_parse_name(const std::string &name) {
    if (name.empty()) return std::make_pair("", "");

    if (name.back() != ')') return std::make_pair(name, "");

    int right = static_cast<int>(name.size()) - 1, left = -1;

    int balance = 0;
    for (int i = static_cast<int>(name.size()) - 1; i >= 0; i--) {
        if (name[i] == '(') balance--;
        else if (name[i] == ')') balance++;

        if (balance == 0) {
            left = i;
            break;
        }
    }

    if (left == -1) return std::make_pair(name, "");

    return std::make_pair(name.substr(0, left), name.substr(left + 1, right - left - 1));
}

} // namespace oink_judge
