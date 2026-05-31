# Tests to Add — shared/python_binding/awaitable_support

## Missing `AwaitableBridge` Lifecycle Tests

### Case 1: `hasCurrent` / `has_current_bridge` reflects activation state

- **Function / Class:** `AwaitableBridge::hasCurrent`
- **Scenario:** Call `has_current_bridge()` (or `AwaitableBridge.hasCurrent()`) before, during, and after an `async with bridge:` block.
- **Why it matters:** `hasCurrent()` is part of the public API exposed to Python but is never asserted in any test. A regression where it always returns `True`/`False` would be invisible.
- **Expected behavior:** Returns `False` before activation, `True` inside `async with`, `False` after exit.

---

### Case 2: `createFuture()` called without active bridge raises `RuntimeError`

- **Function / Class:** `AwaitableBridge::createFuture`
- **Scenario:** Construct an `AwaitableBridge`, do not activate it, then call `bridge.createFuture()` (or call any C++ awaitable function that internally calls `createFuture` outside `async with`).
- **Why it matters:** The error path in `createFuture()` (`cached_loop_ == nullptr`) guards against programming mistakes. It is untested — a regression could silently dereference a null pointer instead of raising a clear `RuntimeError`.
- **Expected behavior:** `RuntimeError` with a descriptive message is raised.

---

### Case 3: `deactivate()` without matching `activate()` raises `RuntimeError`

- **Function / Class:** `AwaitableBridge::deactivate`
- **Scenario:** Create a bridge, never activate it, then call `bridge.deactivate()` directly.
- **Why it matters:** The guard `current_ != this` in `deactivate()` exists to catch misuse. Without a test, a future refactor could break this guard silently.
- **Expected behavior:** `RuntimeError` is raised.

---

### Case 4: Destructor while active (GC / exception path) does not dangle

- **Function / Class:** `AwaitableBridge` destructor
- **Scenario:** Create a bridge, activate it inside a coroutine, then let the bridge object go out of scope (del / replace reference) while still inside `async with`. Verify the thread-local pointer is cleared and `has_current_bridge()` returns `False` afterward.
- **Why it matters:** The destructor calls `deactivate()` with a try/catch to prevent dangling thread-local pointer. This path is exercised only if Python GC destroys a live bridge, which can happen in error scenarios.
- **Expected behavior:** No crash; `has_current_bridge()` returns `False` after the bridge is deleted.

---

## Missing `AwaitableBinder` / `bindAwaitable` Tests

### Case 5: `AwaitableBinder` for a free function (`awaitable<Result>(*)(Args...)`)

- **Function / Class:** `AwaitableBinder<awaitable<Result>(*)(Args...)>`, `bindAwaitable`
- **Scenario:** Wrap a free C++ function returning `awaitable<int>` with `bindAwaitable`, expose it to Python, and call it inside `async with bridge:`.
- **Why it matters:** `AwaitableBinder` is the primary API of `awaitable_binder.hpp` for integrating user-defined coroutines with Python. The free-function specialization is completely untested — user code using `bindAwaitable(&myFreeFunction)` might be broken.
- **Expected behavior:** The returned `py::object` is an awaitable Future; `await`-ing it yields the coroutine's return value.

---

### Case 6: `AwaitableBinder` for a non-const member function

- **Function / Class:** `AwaitableBinder<awaitable<Result>(Class::*)(Args...)>`
- **Scenario:** Wrap a non-const method of a user class with `bindAwaitable`, expose the class to Python with `shared_ptr` holder, and call the method via `async with bridge:`.
- **Why it matters:** This specialization is the typical use case for service objects (e.g., sessions, handlers) that expose async methods to Python. It is untested.
- **Expected behavior:** The wrapper calls the method on the correct `shared_ptr` instance; the Future resolves to the correct return value.

---

### Case 7: `AwaitableBinder` for a const member function

- **Function / Class:** `AwaitableBinder<awaitable<Result>(Class::*)(Args...) const>`
- **Scenario:** Same as Case 6 but with a `const` method.
- **Why it matters:** The `const` specialization is separate code path (`fn` type differs). A missing `const` qualifier on the member function pointer would cause a silent type mismatch; the test ensures the const specialization compiles and works correctly.
- **Expected behavior:** Identical to Case 6 for a `const`-qualified method.

---

### Case 8: `AwaitableBinder` with `void` return type

- **Function / Class:** `awaitAndResolve` (`if constexpr (std::is_void_v<Result>)` branch)
- **Scenario:** Wrap a C++ function returning `awaitable<void>` with `bindAwaitable`, call it from Python, and verify the Future resolves to `None`.
- **Why it matters:** The `if constexpr` branch for `void` in `awaitAndResolve` is a separate code path that sets `set_result(py::none())`. If broken (e.g., accidentally setting a non-None value or throwing), awaiting a void C++ coroutine from Python fails silently or raises.
- **Expected behavior:** `await` on the Future returns `None`.

---

## Missing `awaitable_bridge.py` Python Wrapper Tests

### Case 9: Python wrapper `AwaitableBridge` — full lifecycle with `poll()` direct call

- **Function / Class:** `awaitable_bridge.AwaitableBridge` (Python wrapper in `tests/awaitable_bridge.py`)
- **Scenario:** Import `AwaitableBridge` from `awaitable_bridge.py`, use `async with bridge:` and call `bridge.poll()` manually to verify the `poll()` delegation to the C++ layer works.
- **Why it matters:** The Python wrapper `awaitable_bridge.py` is never imported or tested in any test file. `test_bridge_v2.py` uses `bridge_v2_module.AwaitableBridge` (C++ class) directly, bypassing the Python wrapper. Any bug in the wrapper (e.g., wrong `deactivate()` ordering) goes undetected.
- **Expected behavior:** `async with bridge:` activates the bridge, C++ awaitables complete, `deactivate()` is called on exit.

---

### Case 10: `test_bridge_v2.py` missing `if __name__ == "__main__"` entry point

- **Function / Class:** `test_bridge_v2.py` (test runner)
- **Scenario:** The file defines 8 test functions but has no `if __name__ == "__main__":` block that calls them, unlike `test_bridge.py` and `test_pure_asyncio.py`.
- **Why it matters:** Running `python test_bridge_v2.py` directly produces no output and no failures — all tests are silently skipped. CTest invokes the file directly, so this is a real gap.
- **Expected behavior:** Add an `if __name__ == "__main__":` block that calls all test functions and prints a summary, matching the pattern in `test_bridge.py`.

---

## Missing `AwaitableBridge` Error Propagation Tests

### Case 11: Exception in C++ coroutine propagates to the Python Future

- **Function / Class:** `awaitAndResolve`, `spawnFunction`, `spawnMethod`
- **Scenario:** Expose a C++ coroutine that throws an exception. Await the resulting Future from Python and verify the Future is rejected with the exception (i.e., `await`-ing it raises `RuntimeError` or `Exception`).
- **Why it matters:** Currently `awaitAndResolve` has no try/catch. An uncaught exception inside the coroutine calls `std::terminate` (via `co_spawn` with `detached` token). The test would catch this silent crash scenario and motivate adding `exception_ptr`-based error propagation to the Future via `future.set_exception(...)`.
- **Expected behavior:** Future is rejected; `await` raises the original exception (or a wrapped one). If the current design terminates, the test documents the limitation.
