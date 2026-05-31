# Issues — logger

## Potential Bugs

### Incomplete Default Fallback

#### Issue 1: `getLogLevel` returns 0 for unknown modules instead of falling back to "default"

- **Location:** `src/logger.cpp:57-62`
- **Description:** `getLogLevel` returns a hard-coded `0` when a module is not found in `log_levels_`. It does not consult the `"default"` key. While `isLoggingEnabled` handles the "default" fallback internally, any external caller of `getLogLevel` (including the Python binding `get_log_level`) gets `0` for an unregistered module even when `"default"` is set to a higher value. This is inconsistent — `isLoggingEnabled` treats unregistered modules as having the "default" level, but `getLogLevel` does not.
- **Suggested fix:** Make `getLogLevel` fall back to the `"default"` key when the module is not found: `if (module != "default") { auto def = log_levels_.find("default"); if (def != log_levels_.end()) return def->second; }`. Then simplify `isLoggingEnabled` to just `return level <= getLogLevel(module);`.

---

### Portability

#### Issue 2: `localtime_r` is POSIX-only

- **Location:** `src/logger.cpp:92`
- **Description:** `localtime_r` is a POSIX function and is not available on Windows (MSVC provides `localtime_s` with a different signature). This limits portability.
- **Suggested fix:** Use C++20 `<chrono>` formatting, or add a platform wrapper (`#ifdef _WIN32` → `localtime_s`, else `localtime_r`).

---

## Architecture Issues

### Thread Safety

#### Issue 3: No synchronization in singleton Logger

- **Location:** `Logger` class
- **Description:** `Logger` is a singleton with shared mutable state (`out_stream_`, `log_levels_`, `color_map_`, formatting options) but provides no synchronization. Concurrent calls to `log()`, `setLogLevel()`, `setColorMap()`, or the free-function wrappers from multiple threads cause data races (undefined behaviour).
- **Suggested fix:** Add a `std::mutex` to `Logger` and lock it in `log()`, `print()`, and all setter methods. Alternatively, use `std::shared_mutex` with shared locks in getters and exclusive locks in setters.

---

### Python Integration

#### Issue 4: Python binding missing `log_debug`, `log_warning`, `log_critical` convenience functions

- **Location:** `python_binding/binding.cpp`
- **Description:** The C++ API exposes all six convenience functions (`logDebug`, `logInfo`, `logSuccess`, `logWarning`, `logError`, `logCritical`), but the Python binding only exposes `log_message`, `log_error`, `log_info`, and `log_success`. The `log_debug`, `log_warning`, and `log_critical` wrappers are missing, creating an inconsistent API surface.
- **Suggested fix:** Add `log_debug`, `log_warning`, and `log_critical` bindings following the same pattern as the existing ones.

#### Issue 5: `PythonLogger::log` reimports `logging` module on every call

- **Location:** `src/python_logger.cpp:42`
- **Description:** Every call to `PythonLogger::log` acquires the GIL and calls `py::module_::import("logging")`. While pybind11 caches module imports internally, the GIL acquisition and attribute lookup on every log call adds unnecessary overhead on a hot path.
- **Suggested fix:** Cache the `logging` module and `getLogger` results (e.g., in a `static` local or a per-module map), or at minimum document that the import is cached by pybind11.

