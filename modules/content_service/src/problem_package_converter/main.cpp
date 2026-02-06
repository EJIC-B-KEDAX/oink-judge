#include "oink_judge/content_service/problem_package_converter/package_converter.hpp"

#include <iostream>
#include <oink_judge/config/config.h>

int main(int argc, char** argv) {
    if (argc != 4) {
        std::cout << "Invalid parameters" << '\n';
        return 1;
    }
    std::string package_type = argv[1];    // NOLINT(cppcoreguidelines-pro-bounds-pointer-arithmetic)
    std::string path_to_package = argv[2]; // NOLINT(cppcoreguidelines-pro-bounds-pointer-arithmetic)
    std::string path_to_config = argv[3];  // NOLINT(cppcoreguidelines-pro-bounds-pointer-arithmetic)
    oink_judge::config::Config::setConfigFilePath(path_to_config);
    try {
        auto converter =
            oink_judge::content_service::problem_package_converter::PackageConverterFactory::instance().create(package_type);
        converter->convertPackage(path_to_package);
    } catch (const std::exception& e) {
        std::cerr << "Error: " << e.what() << '\n';
        return 1;
    }
    std::cout << "Package converted successfully" << '\n';
}
