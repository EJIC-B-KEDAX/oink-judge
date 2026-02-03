#pragma once
#include "type_factory.hpp"

namespace oink_judge::factory {

template <smart_or_raw_pointer Base, typename... Args>
auto TypeFactory<Base, Args...>::instance() -> TypeFactory<Base, Args...>& {
    static TypeFactory instance;
    return instance;
}

template <smart_or_raw_pointer Base, typename... Args>
auto TypeFactory<Base, Args...>::getRegisteredTypes() const
    -> const std::unordered_map<std::string, typename TypeFactory<Base, Args...>::CreateFunc>& {
    return registered_types_;
}

template <smart_or_raw_pointer Base, typename... Args>
auto TypeFactory<Base, Args...>::registerType(const std::string& name, CreateFunc func) -> void {
    registered_types_[name] = func;
}

template <smart_or_raw_pointer Base, typename... Args>
auto TypeFactory<Base, Args...>::create(const std::string& name, Args&&... args) const -> Base {
    auto iter = registered_types_.find(name);
    if (iter == registered_types_.end()) {
        throw std::runtime_error("Unknown type: " + name);
    }
    return iter->second(std::forward<Args>(args)...);
}

template <smart_or_raw_pointer Base, typename... Args>
auto TypeFactory<Base, Args...>::accessRegisteredTypes()
    -> std::unordered_map<std::string, typename TypeFactory<Base, Args...>::CreateFunc>& {
    return registered_types_;
}

} // namespace oink_judge::factory
