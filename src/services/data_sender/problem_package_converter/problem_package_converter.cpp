#include "services/data_sender/problem_package_converter/PackageConverter.hpp"
#include "config/Config.h"
#include <iostream>

int main(int argc, char **argv) {
    if (argc != 4) {
        std::cout << "Invalid parameters" << std::endl;
        return 1;
    }
    std::string package_type = argv[1];
    std::string path_to_package = argv[2];
    std::string path_to_config = argv[3];
    oink_judge::config::Config::set_config_file_path(path_to_config);
    try {
        auto converter = oink_judge::services::data_sender::problem_package_converter::PackageConverterFactory::instance().create(package_type);
        converter->convert_package(path_to_package);
    } catch (const std::exception &e) {
        std::cerr << "Error: " << e.what() << std::endl;
        return 1;
    }
    std::cout << "Package converted successfully" << std::endl;
}
