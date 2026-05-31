#include <Python.h>
#include <boost/asio.hpp>

namespace oink_judge::python_bindings::awaitable_support {

// =============================================================================
// AwaitableBridge
//
// Owns one io_context and binds it to a Python asyncio event loop for the
// duration of an `async with bridge:` block.
//
// Lifecycle:
//   1. Python creates: bridge = AwaitableBridge()
//   2. Python activates: async with bridge:  (calls activate / deactivate)
//      - activate() stores the running loop, registers this as thread_local current
//      - deactivate() unregisters, clears loop reference
//   3. While active, any C++ function can call AwaitableBridge::current() to
//      get the bridge — no parameters needed.
//
// Thread safety:
//   The bridge must be activated on the thread that runs the asyncio event loop.
//   thread_local current_ ensures each thread sees only its own active bridge.
//   Using one bridge from multiple threads concurrently is undefined behaviour
//   (io_context has concurrency_hint=1).
// =============================================================================
class AwaitableBridge {
  public:
    AwaitableBridge() = default;

    // Non-copyable, non-movable: coroutine frames hold a reference to our ioc_.
    AwaitableBridge(const AwaitableBridge&) = delete;
    auto operator=(const AwaitableBridge&) -> AwaitableBridge& = delete;
    AwaitableBridge(AwaitableBridge&&) = delete;
    auto operator=(AwaitableBridge&&) -> AwaitableBridge& = delete;

    ~AwaitableBridge() noexcept;

    // -------------------------------------------------------------------------
    // activate() / deactivate()
    // Called from Python's __aenter__ / __aexit__ of the async-with block.
    //
    // activate() caches the running loop so create_future() can avoid calling
    // back into Python on every spawned coroutine.
    // -------------------------------------------------------------------------
    auto activate() -> void;

    auto deactivate() -> void;

    // -------------------------------------------------------------------------
    // poll()
    // Called from the asyncio driver task on every event-loop iteration.
    // -------------------------------------------------------------------------
    auto poll() -> void;

    // -------------------------------------------------------------------------
    // createFuture()
    // Returns a new asyncio.Future bound to the cached event loop.
    // -------------------------------------------------------------------------
    // Returns a new reference. Caller takes ownership.
    auto createFuture() -> PyObject*;

    auto ioContext() -> boost::asio::io_context&;

    // -------------------------------------------------------------------------
    // current()
    // Returns the bridge that is currently active on this thread.
    // Throws if no bridge is active (programming error).
    // -------------------------------------------------------------------------
    static auto current() -> AwaitableBridge&;

    static auto hasCurrent() -> bool;

  private:
    boost::asio::io_context ioc_{1};
    PyObject* cached_loop_{nullptr};

    // Thread-local stack (depth 1 for now — nesting supported via previous_).
    static thread_local AwaitableBridge* current_; // NOLINT
    AwaitableBridge* previous_{nullptr};
};

} // namespace oink_judge::python_bindings::awaitable_support
