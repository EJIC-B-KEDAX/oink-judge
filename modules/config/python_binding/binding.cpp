#include <oink_judge/config/common_utils.h>
#include <oink_judge/config/config.h>
#include <oink_judge/config/logger_utils.h>
#include <oink_judge/config/problem_config_utils.h>
#include <pybind11/chrono.h>
#include <pybind11/detail/common.h>
#include <pybind11/pybind11.h>
#include <pybind11/stl.h>
#include <pybind11/stl/filesystem.h>

using namespace oink_judge;
namespace py = pybind11;

PYBIND11_MODULE(pybind11_config, m) {
    // common_utils
    m.def("get_directory_path", &config::getDirectoryPath);
    m.def("get_timing", &config::getTiming);
    m.def("set_config_file_path", &config::Config::setConfigFilePath);
    m.def("set_credentials_file_path", &config::Config::setCredentialsFilePath);

    // logger_utils
    py::class_<config::LoggerConfig>(m, "LoggerConfig")
        .def(py::init<>())
        .def_readwrite("output_stream", &config::LoggerConfig::output_stream)
        .def_readwrite("log_levels", &config::LoggerConfig::log_levels)
        .def_readwrite("color_map", &config::LoggerConfig::color_map)
        .def_readwrite("min_location_length", &config::LoggerConfig::min_location_length)
        .def_readwrite("min_module_length", &config::LoggerConfig::min_module_length);

    m.def("get_logger_config", &config::getLoggerConfig);
    m.def("get_logger_output_stream", &config::getLoggerOutputStream);
    m.def("get_logger_log_level", &config::getLoggerLogLevel);
    m.def("get_all_logger_log_levels", &config::getAllLoggerLogLevels);
    m.def("get_logger_color_map", &config::getLoggerColorMap);
    m.def("get_logger_min_location_length", &config::getLoggerMinLocationLength);
    m.def("get_logger_min_module_length", &config::getLoggerMinModuleLength);
    m.def("configure_logger", &config::configureLogger);

    // problem_config_utils
    m.def("get_problem_builder_name", &problem_config::getProblemBuilderName);
    m.def("get_all_test_names", &problem_config::getAllTestNames);
    m.def("get_path_to_problem_statements", &problem_config::getPathToProblemStatements);
    m.def("get_problem_statements", &problem_config::getProblemStatements);
}
