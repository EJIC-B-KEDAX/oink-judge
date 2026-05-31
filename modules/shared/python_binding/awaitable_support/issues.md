# Issues — shared/python_binding/awaitable_support

## Potential Bugs

### Reference Counting Errors

#### Issue 1: `Py_INCREF` on `raw_future` before `co_spawn` — no corresponding `Py_DECREF` on exception path

- **Location:** `include/oink_judge/python_binding/awaitable_support/awaitable_binder.inl:38,47`
- **Description:** In `spawnMethod` and `spawnFunction`, `Py_INCREF(raw)` is called and then `co_spawn(…)` is invoked. If `co_spawn` throws (e.g., due to the `io_context` being stopped), the `raw` pointer is leaked — `raw_future`'s refcount was incremented but the coroutine that would `Py_DECREF` it via `reinterpret_steal` will never run.
- **Suggested fix:** Wrap `Py_INCREF` + `co_spawn` in a try/catch that calls `Py_DECREF(raw)` on exception, or use a scope guard (`std::unique_ptr` with a custom deleter).

---

#### Issue 2: `awaitAndResolve` does not hold the GIL when calling `set_result`

- **Location:** `include/oink_judge/python_binding/awaitable_support/awaitable_binder.inl:17,19,28,30`
- **Description:** `awaitAndResolve` calls `py::reinterpret_steal<py::object>(raw_future).attr("set_result")(…)` without acquiring the GIL. If the coroutine resumes on a Boost.Asio thread that does not hold the GIL, accessing Python objects causes undefined behaviour / CPython crashes.
- **Suggested fix:** Add `py::gil_scoped_acquire gil;` at the top of both `awaitAndResolve` overloads before any Python object access.

---

### Resource Leaks on Deactivation

#### Issue 3: `deactivate()` abandons mid-flight coroutines, leaking Python object references

- **Location:** `include/oink_judge/python_binding/awaitable_support/awaitable_bridge.hpp:88`
- **Description:** After `ioc_.stop()` + `ioc_.restart()`, any C++ coroutines mid-flight (e.g., waiting on a timer) are abandoned. Their `PyObject*` raw pointers (`Py_INCREF`'d before `co_spawn`) never have their matching `Py_DECREF` called, leaking Python object references. This happens when the `async with` block exits via exception while a coroutine is suspended.
- **Suggested fix:** Before stopping the io_context, cancel all outstanding work (via `boost::asio::cancellation_signal` or by posting cancellation to each in-flight coroutine) so their frames are destroyed and `Py_DECREF` is triggered. At minimum, document this as a known limitation.

---

## Architecture Issues

### Header-Only Implementation for Non-Template Class

#### Issue 4: `AwaitableBridge` header contains full method bodies despite being a non-template class

- **Location:** `include/oink_judge/python_binding/awaitable_support/awaitable_bridge.hpp`
- **Description:** The header contains the complete class definition, all method bodies, and the `inline thread_local` static member definition. `AwaitableBridge` is not a template — it is a concrete class with Boost.Asio and pybind11 dependencies. Placing everything in a header forces every includer to compile those headers, increasing build times and exposing implementation details.
- **Suggested fix:** Move all method bodies to `src/awaitable_bridge.cpp` and keep only declarations in the header. Remove the `inline thread_local` definition from the header and move it to the `.cpp` file.
