#pragma once
#include "PackageConverter.hpp"

namespace oink_judge::services::data_sender::problem_package_converter {

class PolygonConverter : public PackageConverter {
public:
    PolygonConverter();

    void convert_package(const std::string &path_to_package) override;

    constexpr static auto REGISTERED_NAME = "polygon";

private:
    void convert_icpc_problem_package(const std::string &path_to_package);
};

} // namespace oink_judge::services::data_sender::problem_package_converter
