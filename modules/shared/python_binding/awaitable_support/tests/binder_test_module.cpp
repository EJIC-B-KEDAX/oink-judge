#include <oink_judge/python_binding/awaitable_support/awaitable_binder.hpp>

#include <boost/asio/steady_timer.hpp>
#include <boost/asio/use_awaitable.hpp>
#include <pybind11/pybind11.h>

#include <chrono>
#include <string>

namespace py = pybind11;

using boost::asio::awaitable;
namespace as = oink_judge::python_bindings::awaitable_support;

// =============================================================================
// Free functions for testing AwaitableBinder free-function specialisation
// =============================================================================

auto freeDouble(int value) -> awaitable<int> { co_return value * 2; }

auto freeVoid() -> awaitable<void> { co_return; }

auto freeString(std::string value) -> awaitable<std::string> { co_return value + "_bound"; }

auto freeTimer(int delay_ms, int value) -> awaitable<int> {
    auto executor = co_await boost::asio::this_coro::executor;
    boost::asio::steady_timer timer(executor);
    timer.expires_after(std::chrono::milliseconds(delay_ms));
    co_await timer.async_wait(boost::asio::use_awaitable);
    co_return value;
}

// =============================================================================
// Service class for testing AwaitableBinder member-function specialisations
// =============================================================================

class Counter {
  public:
    explicit Counter(int initial) : value_(initial) {}

    auto increment(int delta) -> awaitable<int> {
        value_ += delta;
        co_return value_;
    }

    auto getValueConst() const -> awaitable<int> { co_return value_; }

    auto reset() -> awaitable<void> {
        value_ = 0;
        co_return;
    }

  private:
    int value_;
};

// =============================================================================
// Module definition
// =============================================================================

PYBIND11_MODULE(binder_test_module, m) {
    m.doc() = "Test module for AwaitableBinder / bindAwaitable template API.";

    // Free functions wrapped with bindAwaitable
    m.def("async_free_double", as::bindAwaitable(&freeDouble), py::arg("value"),
          "Free function returning awaitable<int>: returns value*2.");

    m.def("async_free_void", as::bindAwaitable(&freeVoid), "Free function returning awaitable<void>: returns None.");

    m.def("async_free_string", as::bindAwaitable(&freeString), py::arg("value"),
          "Free function returning awaitable<string>: appends '_bound'.");

    m.def("async_timer", as::bindAwaitable(&freeTimer), py::arg("delay_ms"), py::arg("value"),
          "Waits delay_ms milliseconds then returns value.");

    // Counter class with member functions wrapped with bindAwaitable
    py::class_<Counter, std::shared_ptr<Counter>>(m, "Counter")
        .def(py::init<int>(), py::arg("initial"))
        .def("increment", as::bindAwaitable(&Counter::increment), py::arg("delta"),
             "Non-const method: adds delta to counter and returns new value.")
        .def("get_value_const", as::bindAwaitable(&Counter::getValueConst), "Const method: returns current counter value.")
        .def("reset", as::bindAwaitable(&Counter::reset), "Non-const void method: resets counter to 0.");
}
