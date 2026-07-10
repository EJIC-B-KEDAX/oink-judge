#include <oink_judge/config/common_utils.h>
#include <oink_judge/config/config.h>
#include <oink_judge/config/logger_utils.h>
#include <oink_judge/config/problem_config_utils.h>
#include <oink_judge/python_binding/json_type_caster.hpp>

#include <pybind11/chrono.h>
#include <pybind11/detail/common.h>
#include <pybind11/functional.h>
#include <pybind11/pybind11.h>
#include <pybind11/stl.h>
#include <pybind11/stl/filesystem.h>

using namespace oink_judge;
namespace py = pybind11;

PYBIND11_MODULE(pybind11_config, m) {
    // common_utils
    m.def("get_directory_path", &config::getDirectoryPath);
    m.def("get_timing", &config::getTiming);
    m.def("get_token_from_credentials", &config::getTokenFromCredentials);

    // config
    m.def("config", &config::Config::config);
    m.def("credentials", &config::Config::credentials);
    m.def("set_config_file_path", &config::Config::setConfigFilePath);
    m.def("set_credentials_file_path", &config::Config::setCredentialsFilePath);
    m.def("reload_data", &config::Config::reloadData);

    // check_path_with
    m.def("check_path_with", &config::checkPathWith, py::arg("j"), py::arg("path"), py::arg("predicate"));
    m.def("check_object_is_array", &config::checkObjectIsArray, py::arg("j"), py::arg("path"));
    m.def("check_object_is_binary", &config::checkObjectIsBinary, py::arg("j"), py::arg("path"));
    m.def("check_object_is_boolean", &config::checkObjectIsBoolean, py::arg("j"), py::arg("path"));
    m.def("check_object_is_discarded", &config::checkObjectIsDiscarded, py::arg("j"), py::arg("path"));
    m.def("check_object_is_null", &config::checkObjectIsNull, py::arg("j"), py::arg("path"));
    m.def("check_object_is_number", &config::checkObjectIsNumber, py::arg("j"), py::arg("path"));
    m.def("check_object_is_number_float", &config::checkObjectIsNumberFloat);
    m.def("check_object_is_number_integer", &config::checkObjectIsNumberInteger, py::arg("j"), py::arg("path"));
    m.def("check_object_is_number_unsigned", &config::checkObjectIsNumberUnsigned, py::arg("j"), py::arg("path"));
    m.def("check_object_is_object", &config::checkObjectIsObject, py::arg("j"), py::arg("path"));
    m.def("check_object_is_primitive", &config::checkObjectIsPrimitive, py::arg("j"), py::arg("path"));
    m.def("check_object_is_string", &config::checkObjectIsString, py::arg("j"), py::arg("path"));

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
