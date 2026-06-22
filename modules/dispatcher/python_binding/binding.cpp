#include <oink_judge/dispatcher/client/config_utils.h>

#include <pybind11/pybind11.h>
#include <pybind11/stl.h>

namespace py = pybind11;

PYBIND11_MODULE(pybind11_dispatcher, m) {
    m.def("get_dispatcher_stub_type", &oink_judge::dispatcher::getDispatcherStubType);
}
