#include "oink_judge/python_binding/awaitable_support/awaitable_bridge.h"

#include <pybind11/pybind11.h>

#include <iostream>

namespace py = pybind11;

namespace oink_judge::python_bindings::awaitable_support {

thread_local AwaitableBridge* AwaitableBridge::current_ = nullptr;

AwaitableBridge::~AwaitableBridge() noexcept {
    // If Python destroys the bridge while it is still active (e.g. due to
    // an exception), deactivate cleanly so the thread-local pointer does not
    // dangle.
    if (current_ == this) {
        try {
            deactivate();
        } catch (...) {
            std::cerr << "AwaitableBridge: exception during destruction while active; suppressing\n";
            // Destructors must not throw. Suppress any exception from
            // deactivate() (e.g. Python errors or runtime_error).
        }
    }
}

auto AwaitableBridge::activate() -> void {
    if (current_ == this) {
        throw std::runtime_error("AwaitableBridge: already active on this thread");
    }
    // Cache the running loop once for the lifetime of the activation.
    // asyncio.get_running_loop() raises RuntimeError if no loop is running,
    // which is the correct behaviour if activate() is called outside a coroutine.
    // Obtain the running loop and keep a raw reference (we own the refcount).
    py::object loop = py::module_::import("asyncio").attr("get_running_loop")();
    cached_loop_ = loop.ptr();
    Py_INCREF(cached_loop_);
    previous_ = current_;
    current_ = this;
}

auto AwaitableBridge::deactivate() -> void {
    if (current_ != this) {
        throw std::runtime_error("AwaitableBridge: not the active bridge on this thread");
    }
    current_ = previous_;
    previous_ = nullptr;
    if (cached_loop_ != nullptr) {
        // Release the reference while the GIL is held (we're in a Python call).
        auto dying = py::reinterpret_steal<py::object>(cached_loop_);
        cached_loop_ = nullptr;
        // `dying` decrefs and releases the loop here.
    }
    // Drain any remaining handlers (e.g. coroutines that were mid-flight when
    // the async-with block exited abnormally).
    ioc_.stop();
    ioc_.restart();
}

auto AwaitableBridge::poll() -> void {
    ioc_.restart();
    ioc_.poll();
}

auto AwaitableBridge::createFuture() -> PyObject* {
    if (cached_loop_ == nullptr) {
        throw std::runtime_error("AwaitableBridge: not active — call activate() first");
    }
    // reinterpret_borrow: we DON'T steal; cached_loop_ remains valid.
    // .release().ptr() transfers the new reference to the caller.
    return py::reinterpret_borrow<py::object>(cached_loop_).attr("create_future")().release().ptr();
}

auto AwaitableBridge::ioContext() -> boost::asio::io_context& { return ioc_; }

auto AwaitableBridge::current() -> AwaitableBridge& {
    if (current_ == nullptr) {
        throw std::runtime_error("AwaitableBridge: no active bridge on this thread. "
                                 "Did you forget `async with bridge:`?");
    }
    return *current_;
}

auto AwaitableBridge::hasCurrent() -> bool { return current_ != nullptr; }

} // namespace oink_judge::python_bindings::awaitable_support