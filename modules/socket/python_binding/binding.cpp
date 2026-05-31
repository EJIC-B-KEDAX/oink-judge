#include <oink_judge/socket/client_config_utils.h>
#include <oink_judge/socket/server_config_utils.h>
#include <pybind11/detail/common.h>
#include <pybind11/pybind11.h>
#include <pybind11/stl.h>
#include <pybind11/stl/filesystem.h>

using namespace oink_judge;
namespace py = pybind11;

PYBIND11_MODULE(pybind11_socket, m) {
    // server_config_utils
    m.def("get_my_port", &socket::getMyPort);
    m.def("get_connection_handler_type", &socket::getConnectionHandlerType);

    // client_config_utils
    m.def("get_server_port", &socket::getServerPort);
    m.def("get_server_hostname", &socket::getServerHostname);
    m.def("get_session_type", &socket::getSessionType);
    m.def("get_start_message", &socket::getStartMessage);
    py::class_<socket::ConnectionConfig>(m, "ConnectionConfig")
        .def(py::init<>())
        .def_readwrite("host", &socket::ConnectionConfig::host)
        .def_readwrite("port", &socket::ConnectionConfig::port)
        .def_readwrite("session_type", &socket::ConnectionConfig::session_type)
        .def_readwrite("start_message", &socket::ConnectionConfig::start_message);
    m.def("get_connection_config", &socket::getConnectionConfig);
}
