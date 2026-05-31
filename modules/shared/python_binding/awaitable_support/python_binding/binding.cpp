#include <oink_judge/python_binding/awaitable_support/awaitable_bridge.h>

#include <pybind11/eval.h>
#include <pybind11/pybind11.h>

namespace py = pybind11;
using oink_judge::python_bindings::awaitable_support::AwaitableBridge;

PYBIND11_MODULE(pybind11_awaitable_support, m) {
    m.doc() = "AwaitableBridge v2: explicit Bridge object with thread-local registration.\n"
              "\n"
              "Usage:\n"
              "    bridge = AwaitableBridge()\n"
              "\n"
              "    async def main():\n"
              "        async with bridge:\n"
              "            result = await async_immediate(21)  # -> 42\n";

    py::class_<AwaitableBridge>(m, "AwaitableBridge", py::dynamic_attr())
        .def(py::init<>())
        .def("activate", &AwaitableBridge::activate,
             "Register this bridge as current for this thread. "
             "Must be called from inside a running event loop.")
        .def("deactivate", &AwaitableBridge::deactivate, "Unregister this bridge and release the cached event loop reference.")
        .def("poll", &AwaitableBridge::poll,
             "Run all ready Boost.Asio handlers (non-blocking). "
             "Call from the asyncio driver task on every event-loop iteration.");

    m.def("has_current_bridge", &AwaitableBridge::hasCurrent, "Return True if a bridge is currently active on this thread.");

    // Patch the async context manager protocol onto AwaitableBridge.
    // async def methods cannot be expressed directly in pybind11, so we inject
    // them from a string literal.  dynamic_attr() lets the instance store
    // _driver_task without a pre-declared C++ field.
    py::exec(R"py(
import asyncio as _asyncio

async def __aenter__(self):
    self.activate()
    self._driver_task = _asyncio.create_task(self._driver(), name="asio_driver")
    return self

async def __aexit__(self, exc_type, exc_val, exc_tb):
    if self._driver_task is not None:
        self._driver_task.cancel()
        try:
            await self._driver_task
        except _asyncio.CancelledError:
            pass
        self._driver_task = None
    self.deactivate()
    return False

async def _driver(self):
    while True:
        self.poll()
        await _asyncio.sleep(0)

AwaitableBridge.__aenter__ = __aenter__
AwaitableBridge.__aexit__  = __aexit__
AwaitableBridge._driver    = _driver
)py",
             m.attr("__dict__"));
}
