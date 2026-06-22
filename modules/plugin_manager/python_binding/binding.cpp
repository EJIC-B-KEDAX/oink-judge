#include <oink_judge/plugin_manager/config_utils.h>
#include <oink_judge/plugin_manager/plugin_manager.h>

#include <pybind11/pybind11.h>
#include <pybind11/stl.h>
#include <pybind11/stl/filesystem.h>

namespace py = pybind11;

PYBIND11_MODULE(pybind11_plugin_manager, m) {
    py::class_<oink_judge::plugin_manager::PluginManager>(m, "PluginManager")
        .def(py::init<>())
        .def("load", &oink_judge::plugin_manager::PluginManager::load, py::arg("path"));

    m.def("get_all_plugin_paths", &oink_judge::plugin_manager::getAllPluginPaths);
}
