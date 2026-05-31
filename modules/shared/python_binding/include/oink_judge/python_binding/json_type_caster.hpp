#pragma once
#include <nlohmann/json.hpp>
#include <pybind11/pybind11.h>
#include <pybind11/stl.h>

namespace py = pybind11;

namespace pybind11::detail {

template <> struct type_caster<nlohmann::json> {
    PYBIND11_TYPE_CASTER(nlohmann::json, _("json"));

    auto load(handle src, bool convert) -> bool {
        try {
            value = nlohmann::json::parse(py::str(py::module_::import("json").attr("dumps")(src)).cast<std::string>());
            return true;
        } catch (...) {
            return false;
        }
    }

    static auto cast(const nlohmann::json& src, return_value_policy policy, handle parent) -> handle {
        return py::module_::import("json").attr("loads")(src.dump()).release();
    }
};

} // namespace pybind11::detail
