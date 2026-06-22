#pragma once
#include <oink_judge/factory/parameterized_type_factory.hpp>

#include <filesystem>

namespace oink_judge::content_service::problem_package_converter {

namespace fs = std::filesystem;

class PackageConverter {
  public:
    PackageConverter(const PackageConverter&) = delete;
    auto operator=(const PackageConverter&) -> PackageConverter& = delete;
    PackageConverter(PackageConverter&&) = delete;
    auto operator=(PackageConverter&&) -> PackageConverter& = delete;
    virtual ~PackageConverter() = default;

    virtual auto convertPackage(const fs::path& path_to_package) -> void = 0;

  protected:
    PackageConverter() = default;
};

using PackageConverterFactory = factory::ParameterizedTypeFactory<std::shared_ptr<PackageConverter>>;

} // namespace oink_judge::content_service::problem_package_converter
