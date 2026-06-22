#pragma once
#include <boost/asio/awaitable.hpp>
#include <pybind11/pybind11.h>

namespace oink_judge::python_bindings::awaitable_support {

namespace py = pybind11;
using boost::asio::awaitable;

template <typename Result>
auto awaitAndResolve(PyObject* raw_future, py::object owner_py, awaitable<Result> task) -> awaitable<void>;
template <typename Result> auto awaitAndResolve(PyObject* raw_future, awaitable<Result> task) -> awaitable<void>;

template <typename Result> auto spawnMethod(py::object owner_py, awaitable<Result> task) -> py::object;

template <typename Result> auto spawnFunction(awaitable<Result> task) -> py::object;

template <typename Coro> struct AwaitableBinder;

template <typename Coro> auto bindAwaitable(Coro coro) -> AwaitableBinder<Coro>;

} // namespace oink_judge::python_bindings::awaitable_support

#include "awaitable_binder.inl"
