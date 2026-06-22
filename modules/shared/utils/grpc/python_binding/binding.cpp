#include <oink_judge/utils/grpc/config_utils.h>

#include <pybind11/detail/common.h>
#include <pybind11/pybind11.h>
#include <pybind11/stl.h>

using namespace oink_judge;
namespace py = pybind11;

PYBIND11_MODULE(pybind11_utils_grpc, m) {
    m.def("get_my_endpoint", &utils::grpc::getMyEndpoint);
    m.def("get_server_credentials_type", &utils::grpc::getServerCredentialsType);
    m.def("get_server_interceptor_types", &utils::grpc::getServerInterceptorTypes);
    m.def("get_server_channel_arguments", &utils::grpc::getServerChannelArguments);
    m.def("get_channel_arguments_list", &utils::grpc::getChannelArgumentsList);
}
