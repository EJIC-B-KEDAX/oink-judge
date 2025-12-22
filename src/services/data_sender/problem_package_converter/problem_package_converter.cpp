#include "services/data_sender/problem_package_converter/PackageConverter.hpp"
#include <iostream>

int main(int argc, char **argv) {
    if (argc != 2) {
        std::cout << "Invalid parameters" << std::endl;
        return 1;
    }
    std::string package_type = argv[0];
    std::string path_to_package = argv[1];
    try {
        auto converter = oink_judge::services::data_sender::problem_package_converter::PackageConverterFactory::instance().create(package_type);
        converter->convert_package(path_to_package);
    } catch (const std::exception &e) {
        std::cerr << "Error: " << e.what() << std::endl;
        return 1;
    }
    std::cout << "Package converted successfully" << std::endl;
}
