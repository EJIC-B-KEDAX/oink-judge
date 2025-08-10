#pragma once
#include "TypeFactory.h"

namespace oink_judge {

template<smart_or_raw_pointer Base, typename... Args>
TypeFactory<Base, Args...> &TypeFactory<Base, Args...>::instance() {
    static TypeFactory instance;
    return instance;
}

template<smart_or_raw_pointer Base, typename... Args>
const std::unordered_map<std::string, typename TypeFactory<Base, Args...>::CreateFunc> &TypeFactory<Base, Args...>::get_registered_types() const {
    return _registered_types;
}

template<smart_or_raw_pointer Base, typename... Args>
void TypeFactory<Base, Args...>::register_type(const std::string &name, CreateFunc func) {
    _registered_types[name] = func;
}

template<smart_or_raw_pointer Base, typename... Args>
Base TypeFactory<Base, Args...>::create(const std::string &name, Args &&... args) const {
    auto it = _registered_types.find(name);
    if (it == _registered_types.end()) {
        throw std::runtime_error("Unknown type: " + name);
    }
    return it->second(std::forward<Args>(args)...);
}

template<smart_or_raw_pointer Base, typename... Args>
std::unordered_map<std::string, typename TypeFactory<Base, Args...>::CreateFunc> &TypeFactory<Base, Args...>::access_registered_types() {
    return _registered_types;
}

} // namespace oink_judge