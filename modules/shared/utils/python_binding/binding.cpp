#include <oink_judge/python_binding/perms_type_caster.hpp>
#include <oink_judge/utils/crypto.h>
#include <oink_judge/utils/filesystem.h>

#include <pybind11/detail/common.h>
#include <pybind11/pybind11.h>
#include <pybind11/stl.h>
#include <pybind11/stl/filesystem.h>

using namespace oink_judge;
namespace py = pybind11;

PYBIND11_MODULE(pybind11_utils, m) {
    // crypto
    m.def("sha256", &utils::crypto::sha256, py::arg("input"));
    m.def("to_base64", &utils::crypto::toBase64, py::arg("input"));
    m.def("from_base64", &utils::crypto::fromBase64, py::arg("input"));

    // filesystem
    m.def("load_file", &utils::filesystem::loadFile, py::arg("path"));
    m.def("store_file", &utils::filesystem::storeFile, py::arg("path"), py::arg("content"));
    m.def("create_directory_if_not_exists", &utils::filesystem::createDirectoryIfNotExists, py::arg("path"));
    m.def("create_file_if_not_exists", &utils::filesystem::createFileIfNotExists, py::arg("path"));
    m.def("pack_directory_to_zip", &utils::filesystem::packDirectoryToZip, py::arg("directory_path"), py::arg("zip_path"));
    m.def("unpack_zip_to_directory", &utils::filesystem::unpackZipToDirectory, py::arg("zip_path"), py::arg("directory_path"));
    m.def("remove_file_or_directory", &utils::filesystem::removeFileOrDirectory, py::arg("path"));
    m.def("clear_directory", &utils::filesystem::clearDirectory, py::arg("path"));
    m.def("set_permissions", &utils::filesystem::setPermissions, py::arg("path"), py::arg("permissions"));
    m.def("get_permissions", &utils::filesystem::getPermissions, py::arg("path"));
}
