#include <oink_judge/factory/parameterized_type_factory.hpp>

#include <pybind11/cast.h>
#include <pybind11/detail/common.h>
#include <pybind11/pybind11.h>
#include <pybind11/stl.h>

using namespace oink_judge;
namespace py = pybind11;

PYBIND11_MODULE(pybind11_factory, m) {
    // strings transform utils
    m.def("parse_name", &factory::parseName, py::arg("name"));
    m.def("parse_parameters", &factory::parseParameters, py::arg("params"), py::arg("delimiters"));
    m.def("normalize_argument", &factory::normalizeArgument, py::arg("name"), py::arg("remove_whitespaces") = false);
}
