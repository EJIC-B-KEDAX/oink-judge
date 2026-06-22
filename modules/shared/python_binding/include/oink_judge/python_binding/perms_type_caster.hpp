#pragma once
#include <pybind11/pybind11.h>
#include <pybind11/stl.h>

#include <filesystem>

namespace py = pybind11;
namespace fs = std::filesystem;

namespace pybind11::detail {

template <> struct type_caster<fs::perms> {
    PYBIND11_TYPE_CASTER(fs::perms, const_name("perms"));

    auto load(handle src, bool convert) -> bool {
        try {
            value = static_cast<fs::perms>(src.cast<int>());
            return true;
        } catch (...) {
            return false;
        }
    }

    static auto cast(const fs::perms& src, return_value_policy policy, handle parent) -> handle {
        return py::int_(static_cast<int>(src)).release();
    }
};

} // namespace pybind11::detail
