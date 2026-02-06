#pragma once
#include <oink_judge/factory/parameterized_type_factory.hpp>
#include <string>

namespace oink_judge::content_service::problem_package_converter {

class PackageConverter {
  public:
    PackageConverter(const PackageConverter&) = delete;
    auto operator=(const PackageConverter&) -> PackageConverter& = delete;
    PackageConverter(PackageConverter&&) = delete;
    auto operator=(PackageConverter&&) -> PackageConverter& = delete;
    virtual ~PackageConverter() = default;

    virtual void convertPackage(const std::string& path_to_package) = 0;

  protected:
    PackageConverter() = default;
};

using PackageConverterFactory = factory::ParameterizedTypeFactory<std::shared_ptr<PackageConverter>>;

} // namespace oink_judge::content_service::problem_package_converter
