#pragma once
#include <functional>
#include <memory>

namespace oink_judge::factory {

template <typename T>
concept smart_or_raw_pointer =
    (requires { typename T::element_type; } && std::same_as<T, std::shared_ptr<typename T::element_type>> ||
     std::same_as<T, std::unique_ptr<typename T::element_type>>) ||
    std::is_pointer_v<T>;

template <smart_or_raw_pointer Base, typename... Args> class TypeFactory {
  public:
    using CreateFunc = std::function<Base(Args...)>;

    TypeFactory(const TypeFactory&) = delete;
    auto operator=(const TypeFactory&) -> TypeFactory& = delete;
    TypeFactory(TypeFactory&&) = delete;
    auto operator=(TypeFactory&&) -> TypeFactory& = delete;
    virtual ~TypeFactory() = default;

    static auto instance() -> TypeFactory&;

    auto getRegisteredTypes() const -> const std::unordered_map<std::string, CreateFunc>&;

    virtual auto registerType(const std::string& name, CreateFunc func) -> void;

    virtual auto create(const std::string& name, Args&&... args) const -> Base;

  protected:
    TypeFactory() = default;

    auto accessRegisteredTypes() -> std::unordered_map<std::string, CreateFunc>&;

  private:
    std::unordered_map<std::string, CreateFunc> registered_types_;
};

} // namespace oink_judge::factory

#include "type_factory.inl"
