#pragma once
#include "type_factory.hpp"

namespace oink_judge::factory {

template <smart_or_raw_pointer Base, typename... Args>
class ParameterizedTypeFactory : public TypeFactory<Base, const std::string&, Args...> {
  public:
    static auto instance() -> ParameterizedTypeFactory&;

    auto create(const std::string& name, Args&&... args) const -> Base;

  protected:
    static auto parseName(const std::string& name) -> std::pair<std::string, std::string>;
};

} // namespace oink_judge::factory

#include "parameterized_type_factory.inl"
