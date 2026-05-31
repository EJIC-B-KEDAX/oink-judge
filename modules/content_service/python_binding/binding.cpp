#include <oink_judge/content_service/config_utils.h>
#include <oink_judge/content_service/content_manifest.h>
#include <oink_judge/content_service/manifest_storage.h>
#include <oink_judge/python_binding/json_type_caster.hpp>

#include <pybind11/cast.h>
#include <pybind11/detail/common.h>
#include <pybind11/pybind11.h>
#include <pybind11/stl.h>
#include <pybind11/stl/filesystem.h>

using namespace oink_judge;
namespace py = pybind11;

PYBIND11_MODULE(pybind11_content_service, m) {
    // config_utils
    m.def("get_content_directory", &content_service::getContentDirectory, py::arg("content_type"));

    // content_manifest
    py::class_<content_service::ContentManifest>(m, "ContentManifest")
        .def(py::init<std::string, std::string>(), py::arg("content_type"), py::arg("content_id"))
        .def("get_content_type", &content_service::ContentManifest::getContentType)
        .def("get_content_id", &content_service::ContentManifest::getContentId)
        .def("to_string", &content_service::ContentManifest::toString)
        .def("to_json", &content_service::ContentManifest::toJson)
        .def("get_path_to_manifest_file", &content_service::ContentManifest::getPathToManifestFile);

    m.def("get_manifest_signature", &content_service::getManifestSignature, py::arg("content_type"), py::arg("content_id"));

    py::enum_<content_service::ContentChange::Type>(m, "ContentChangeType")
        .value("ADDED", content_service::ContentChange::Type::ADDED)
        .value("REMOVED", content_service::ContentChange::Type::REMOVED)
        .value("MODIFIED", content_service::ContentChange::Type::MODIFIED)
        .value("ATTRIBUTES_CHANGED", content_service::ContentChange::Type::ATTRIBUTES_CHANGED)
        .export_values();

    py::class_<content_service::ContentChange>(m, "ContentChange")
        .def_readwrite("type", &content_service::ContentChange::type)
        .def_readwrite("file_path", &content_service::ContentChange::file_path);

    m.def("_compare_manifests_manifest_json",
          py::overload_cast<const content_service::ContentManifest&, const nlohmann::json&>(&content_service::compareManifests),
          py::arg("old_manifest"), py::arg("new_manifest"));
    m.def("_compare_manifests_json_json",
          py::overload_cast<const nlohmann::json&, const nlohmann::json&>(&content_service::compareManifests),
          py::arg("old_manifest"), py::arg("new_manifest"));

    // manifest_storage
    py::class_<content_service::ManifestStorage>(m, "ManifestStorage")
        .def_static("instance", &content_service::ManifestStorage::instance, py::return_value_policy::reference)
        .def("get_manifest", &content_service::ManifestStorage::getManifest, py::arg("content_type"), py::arg("content_id"),
             py::return_value_policy::reference);
}
