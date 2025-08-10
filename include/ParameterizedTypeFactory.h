#pragma once
#include <memory>
#include "TypeFactory.h"

namespace oink_judge {

template<smart_or_raw_pointer Base, typename... Args>
class ParameterizedTypeFactory : public TypeFactory<Base, const std::string&, Args...> {
public:
    static ParameterizedTypeFactory& instance();

    Base create(const std::string &name, Args&&... args) const;

protected:
    static std::pair<std::string, std::string> _parse_name(const std::string &name);
};

} // namespace oink_judge

#include "ParameterizedTypeFactory.inl"
