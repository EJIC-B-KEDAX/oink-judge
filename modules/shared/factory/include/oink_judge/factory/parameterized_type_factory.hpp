#pragma once
#include "type_factory.hpp"

namespace oink_judge::factory {

template <smart_or_raw_pointer Base, typename... Args>
class ParameterizedTypeFactory : public TypeFactory<Base, const std::string&, Args...> {
  public:
    static auto instance() -> ParameterizedTypeFactory&;

    auto create(const std::string& name, Args&&... args) const -> Base;
};

inline auto parseName(const std::string& name) -> std::pair<std::string, std::string>;
inline auto parseParameters(const std::string& params, const std::string& delimiters) -> std::vector<std::string>;
inline auto normalizeArgument(const std::string& name, bool remove_whitespaces = false) -> std::string;

} // namespace oink_judge::factory

#include "parameterized_type_factory.inl"
