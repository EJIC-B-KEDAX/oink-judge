#pragma once
#include <functional>
#include <memory>

namespace oink_judge {

template <typename T>
concept smart_or_raw_pointer =
    (requires { typename T::element_type; } &&
    std::same_as<T, std::shared_ptr<typename T::element_type>> ||
    std::same_as<T, std::unique_ptr<typename T::element_type>>) ||
    std::is_pointer_v<T>;

template<smart_or_raw_pointer Base, typename... Args>
class TypeFactory {
public:
    using CreateFunc = std::function<Base(Args...)>;

    virtual ~TypeFactory() = default;

    static TypeFactory& instance();

    const std::unordered_map<std::string, CreateFunc> &get_registered_types() const;

    virtual void register_type(const std::string &name, CreateFunc func);

    virtual Base create(const std::string &name, Args&&... args) const;

protected:
    std::unordered_map<std::string, CreateFunc> &access_registered_types();

private:
    std::unordered_map<std::string, CreateFunc> _registered_types;
};

} // namespace oink_judge

#include "TypeFactory.inl"
