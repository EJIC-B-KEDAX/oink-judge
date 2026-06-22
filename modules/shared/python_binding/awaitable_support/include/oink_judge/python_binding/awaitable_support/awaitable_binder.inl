#pragma once
#include "awaitable_binder.hpp"

#include "awaitable_bridge.h"

#include <boost/asio/detached.hpp>
#include <pybind11/gil.h>
#include <pybind11/pytypes.h>

#include <exception>
#include <functional>
#include <utility>

namespace oink_judge::python_bindings::awaitable_support {

using boost::asio::awaitable;
using boost::asio::detached;

namespace detail {

inline auto setFutureException(PyObject* raw_future, std::exception_ptr exception) -> void {
    py::gil_scoped_acquire gil;
    auto future = py::reinterpret_steal<py::object>(raw_future);
    const py::object runtime_error = py::module_::import("builtins").attr("RuntimeError");

    try {
        std::rethrow_exception(std::move(exception));
    } catch (const std::exception& error) {
        future.attr("set_exception")(runtime_error(error.what()));
    } catch (...) {
        future.attr("set_exception")(runtime_error("unknown C++ exception"));
    }
}

} // namespace detail

template <typename Result>
auto awaitAndResolve(PyObject* raw_future, py::object owner_py, awaitable<Result> task) -> awaitable<void> {
    (void)owner_py;
    try {
        if constexpr (std::is_void_v<Result>) {
            co_await std::move(task);
            py::gil_scoped_acquire gil;
            py::reinterpret_steal<py::object>(raw_future).attr("set_result")(py::none());
        } else {
            auto result = co_await std::move(task);
            py::gil_scoped_acquire gil;
            py::reinterpret_steal<py::object>(raw_future).attr("set_result")(py::cast(std::move(result)));
        }
    } catch (...) {
        detail::setFutureException(raw_future, std::current_exception());
    }
    co_return;
}

template <typename Result> auto awaitAndResolve(PyObject* raw_future, awaitable<Result> task) -> awaitable<void> {
    try {
        if constexpr (std::is_void_v<Result>) {
            co_await std::move(task);
            py::gil_scoped_acquire gil;
            py::reinterpret_steal<py::object>(raw_future).attr("set_result")(py::none());
        } else {
            auto result = co_await std::move(task);
            py::gil_scoped_acquire gil;
            py::reinterpret_steal<py::object>(raw_future).attr("set_result")(py::cast(std::move(result)));
        }
    } catch (...) {
        detail::setFutureException(raw_future, std::current_exception());
    }
    co_return;
}

template <typename Result> auto spawnMethod(py::object owner_py, awaitable<Result> task) -> py::object {
    auto& bridge = AwaitableBridge::current();
    auto future = py::reinterpret_steal<py::object>(bridge.createFuture());
    PyObject* raw = future.ptr();
    Py_INCREF(raw);
    co_spawn(bridge.ioContext(), awaitAndResolve(raw, std::move(owner_py), std::move(task)), detached);
    return future;
}

template <typename Result> auto spawnFunction(awaitable<Result> task) -> py::object {
    auto& bridge = AwaitableBridge::current();
    auto future = py::reinterpret_steal<py::object>(bridge.createFuture());
    PyObject* raw = future.ptr();
    Py_INCREF(raw);
    co_spawn(bridge.ioContext(), awaitAndResolve(raw, std::move(task)), detached);
    return future;
}

// pybind11 passes Class& as self for any holder type (unique_ptr, shared_ptr, etc.).
template <typename Result, typename Class, typename... Args> struct AwaitableBinder<awaitable<Result> (Class::*)(Args...)> {
    awaitable<Result> (Class::*fn_)(Args...);

    auto operator()(Class& self, Args... args) const -> py::object {
        return spawnMethod(py::cast(std::ref(self)), (self.*fn_)(std::forward<Args>(args)...));
    }
};

template <typename Result, typename Class, typename... Args> struct AwaitableBinder<awaitable<Result> (Class::*)(Args...) const> {
    awaitable<Result> (Class::*fn_)(Args...) const;

    auto operator()(const Class& self, Args... args) const -> py::object {
        return spawnMethod(py::cast(std::cref(self)), (self.*fn_)(std::forward<Args>(args)...));
    }
};

template <typename Result, typename... Args> struct AwaitableBinder<awaitable<Result> (*)(Args...)> {
    awaitable<Result> (*fn_)(Args...);

    auto operator()(Args... args) const -> py::object { return spawnFunction(fn_(std::forward<Args>(args)...)); }
};

template <typename Coro> auto bindAwaitable(Coro coro) -> AwaitableBinder<Coro> { return AwaitableBinder<Coro>{coro}; }

} // namespace oink_judge::python_bindings::awaitable_support
