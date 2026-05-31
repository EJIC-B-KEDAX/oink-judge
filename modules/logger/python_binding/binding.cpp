#include <oink_judge/logger/logger.h>
#include <oink_judge/logger/python_logger.h>

#include <pybind11/pybind11.h>
#include <pybind11/stl.h>

using namespace oink_judge::logger;
namespace py = pybind11;

PYBIND11_MODULE(pybind11_logger, m) {
    py::enum_<LogType>(m, "LogType")
        .value("DEBUG", LogType::DEBUG)
        .value("INFO", LogType::INFO)
        .value("SUCCESS", LogType::SUCCESS)
        .value("WARNING", LogType::WARNING)
        .value("ERROR", LogType::ERROR)
        .value("CRITICAL", LogType::CRITICAL);

    py::class_<LocationFormatOptions>(m, "LocationFormatOptions")
        .def(py::init<>())
        .def_readwrite("strip_params_on_level", &LocationFormatOptions::strip_params_on_level)
        .def_readwrite("strip_return_type_on_level", &LocationFormatOptions::strip_return_type_on_level)
        .def_readwrite("strip_namespace_on_level", &LocationFormatOptions::strip_namespace_on_level);

    py::class_<Logger>(m, "Logger")
        .def_static("instance", &Logger::instance, py::return_value_policy::reference)
        .def("set_log_level", &Logger::setLogLevel)
        .def("set_color_map", &Logger::setColorMap)
        .def("set_min_location_length", &Logger::setMinLocationLength)
        .def("set_min_module_length", &Logger::setMinModuleLength)
        .def("set_timestamp_format", &Logger::setTimestampFormat)
        .def("set_location_format_options", &Logger::setLocationFormatOptions)
        .def("get_log_level", &Logger::getLogLevel)
        .def("get_color_map", &Logger::getColorMap)
        .def("get_min_location_length", &Logger::getMinLocationLength)
        .def("get_min_module_length", &Logger::getMinModuleLength)
        .def("get_timestamp_format", &Logger::getTimestampFormat)
        .def("get_location_format_options", &Logger::getLocationFormatOptions)
        .def_property_readonly_static("DEFAULT_MIN_LOCATION_LENGTH",
                                      [](const py::object& /*self*/) -> uint32_t { return Logger::DEFAULT_MIN_LOCATION_LENGTH; })
        .def_property_readonly_static("DEFAULT_MIN_MODULE_LENGTH",
                                      [](const py::object& /*self*/) -> uint32_t { return Logger::DEFAULT_MIN_MODULE_LENGTH; });

    m.def("disable_colors", &disableColors);
    m.def("enable_colors", &enableColors, py::arg("color_map") = Logger::DEFAULT_COLOR_MAP);
    m.def(
        "log_message",
        [](const std::string& module, const std::string& message, LogType type, uint32_t level) -> void {
            logMessage(module, message, type, level);
        },
        py::arg("module"), py::arg("message"), py::arg("type") = LogType::INFO, py::arg("level") = 1);
    m.def(
        "log_error",
        [](const std::string& module, const std::string& message, uint32_t level) -> void { logError(module, message, level); },
        py::arg("module"), py::arg("message"), py::arg("level") = 1);
    m.def(
        "log_info",
        [](const std::string& module, const std::string& message, uint32_t level) -> void { logInfo(module, message, level); },
        py::arg("module"), py::arg("message"), py::arg("level") = 1);
    m.def(
        "log_success",
        [](const std::string& module, const std::string& message, uint32_t level) -> void { logSuccess(module, message, level); },
        py::arg("module"), py::arg("message"), py::arg("level") = 1);
}
