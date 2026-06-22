#pragma once
#include "oink_judge/content_service/problem_package_converter/package_converter.hpp"

namespace oink_judge::content_service::problem_package_converter {

class PolygonConverter : public PackageConverter {
  public:
    PolygonConverter();

    auto convertPackage(const fs::path& path_to_package) -> void override;

    constexpr static auto REGISTERED_NAME = "polygon";

  private:
    auto convertICPCProblemPackage(const fs::path& path_to_package) -> void;
};

} // namespace oink_judge::content_service::problem_package_converter
