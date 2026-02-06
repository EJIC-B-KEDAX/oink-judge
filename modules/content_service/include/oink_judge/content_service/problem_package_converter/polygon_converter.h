#pragma once
#include "oink_judge/content_service/problem_package_converter/package_converter.hpp"

namespace oink_judge::content_service::problem_package_converter {

class PolygonConverter : public PackageConverter {
  public:
    PolygonConverter();

    void convertPackage(const std::string& path_to_package) override;

    constexpr static auto REGISTERED_NAME = "polygon";

  private:
    void convertICPCProblemPackage(const std::string& path_to_package);
};

} // namespace oink_judge::content_service::problem_package_converter
