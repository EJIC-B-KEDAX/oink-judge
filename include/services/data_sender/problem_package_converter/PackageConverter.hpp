#pragma once
#include "ParameterizedTypeFactory.hpp"

#include <string>

namespace oink_judge::services::data_sender::problem_package_converter {

class PackageConverter {
  public:
    virtual ~PackageConverter() = default;

    virtual void convert_package(const std::string& path_to_package) = 0;
};

using PackageConverterFactory = ParameterizedTypeFactory<std::shared_ptr<PackageConverter>>;

} // namespace oink_judge::services::data_sender::problem_package_converter
