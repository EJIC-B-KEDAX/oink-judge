#pragma once
#include "awaitable_binder.hpp"

#include "awaitable_bridge.h"

#include <boost/asio/detached.hpp>
#include <pybind11/pytypes.h>

namespace oink_judge::python_bindings::awaitable_support {

using boost::asio::awaitable;
using boost::asio::detached;

template <typename Owner, typename Result>
auto awaitAndResolve(PyObject* raw_future, std::shared_ptr<Owner> owner, awaitable<Result> task) -> awaitable<void> {
    if constexpr (std::is_void_v<Result>) {
        co_await std::move(task);
        py::reinterpret_steal<py::object>(raw_future).attr("set_result")(py::none());
    } else {
        auto result = co_await std::move(task);
        py::reinterpret_steal<py::object>(raw_future).attr("set_result")(py::cast(result));
    }
    co_return;
}

template <typename Result> auto awaitAndResolve(PyObject* raw_future, awaitable<Result> task) -> awaitable<void> {
    if constexpr (std::is_void_v<Result>) {
        co_await std::move(task);
        py::reinterpret_steal<py::object>(raw_future).attr("set_result")(py::none());
    } else {
        auto result = co_await std::move(task);
        py::reinterpret_steal<py::object>(raw_future).attr("set_result")(py::cast(result));
    }
    co_return;
}

template <typename Owner, typename Result> auto spawnMethod(std::shared_ptr<Owner> owner, awaitable<Result> task) -> py::object {
    py::object future = py::reinterpret_steal<py::object>(AwaitableBridge::current().createFuture());
    PyObject* raw = future.ptr();
    Py_INCREF(raw);
    co_spawn(AwaitableBridge::current().ioContext(), awaitAndResolve(raw, std::move(owner), std::move(task)), detached);
    return future;
}

template <typename Result> auto spawnFunction(awaitable<Result> task) -> py::object {
    py::object future = py::reinterpret_steal<py::object>(AwaitableBridge::current().createFuture());
    PyObject* raw = future.ptr();
    Py_INCREF(raw);
    co_spawn(AwaitableBridge::current().ioContext(), awaitAndResolve(raw, std::move(task)), detached);
    return future;
}

template <typename Result, typename Class, typename... Args> struct AwaitableBinder<awaitable<Result> (Class::*)(Args...)> {
    awaitable<Result> (Class::*fn_)(Args...);

    auto operator()(std::shared_ptr<Class> self, Args... args) const -> py::object {
        return spawnMethod(std::move(self), (self.get()->*fn_)(std::forward<Args>(args)...));
    }
};

template <typename Result, typename Class, typename... Args> struct AwaitableBinder<awaitable<Result> (Class::*)(Args...) const> {
    awaitable<Result> (Class::*fn_)(Args...) const;

    auto operator()(std::shared_ptr<Class> self, Args... args) const -> py::object {
        return spawnMethod(std::move(self), (self.get()->*fn_)(std::forward<Args>(args)...));
    }
};

template <typename Result, typename... Args> struct AwaitableBinder<awaitable<Result> (*)(Args...)> {
    awaitable<Result> (*fn_)(Args...);

    auto operator()(Args... args) const -> py::object { return spawnFunction(fn_(std::forward<Args>(args)...)); }
};

template <typename Coro> auto bindAwaitable(Coro coro) -> AwaitableBinder<Coro> { return AwaitableBinder<Coro>{coro}; }

} // namespace oink_judge::python_bindings::awaitable_support
