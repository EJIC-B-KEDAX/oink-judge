# Tests to Add — logger

## C++ Logger — Failing Test for `getLogLevel` Default Fallback

### Case 1: `getLogLevel` should fall back to "default" for unknown modules

- **Function / Class:** `Logger::getLogLevel`
- **Scenario:** Set `"default"` level to 3, call `getLogLevel("nonexistent")`. Currently returns 0 instead of 3.
- **Why it matters:** External callers (including Python's `get_log_level`) get inconsistent results compared to `isLoggingEnabled`, which does fall back to "default". See Issue 1 in `issues.md`.
- **Expected behavior:** Returns 3 (the "default" level). Test will fail until Issue 1 is fixed.

---

## Python Logging Integration — `log_message` Routing

### Case 2: `log_message` routes through Python `logging` module

- **Function / Class:** `log_message` (pybind11 binding)
- **Scenario:** Call `log_message("test_mod", "hello", LogType.INFO, 1)` from Python with a logging handler attached to the `"test_mod"` logger.
- **Why it matters:** The core feature of PythonLogger is routing C++ logs through Python's logging pipeline. If this breaks, C++ modules become silent in Python-hosted processes.
- **Expected behavior:** The message `"hello"` appears in the Python handler's output.

### Case 3: Level filtering applies to Python-routed logs

- **Function / Class:** `log_message` (pybind11 binding)
- **Scenario:** Set module level to 1. Call `log_message` at level 1 (visible) and level 2 (suppressed).
- **Why it matters:** Level filtering uses the C++ `isLoggingEnabled` check before forwarding to Python. If this check is bypassed, all messages flood through regardless of level.
- **Expected behavior:** Level 1 message appears, level 2 message does not.

---

## Python Logging Integration — LogType to Python Level Mapping

### Case 4: DEBUG maps to Python DEBUG (10)

- **Function / Class:** `log_message` / `logTypeToPythonLevel`
- **Scenario:** Log with `LogType.DEBUG` and capture via Python handler at DEBUG level.
- **Why it matters:** Incorrect mapping would hide or miscategorize debug output.
- **Expected behavior:** Python handler records the message at DEBUG level.

### Case 5: INFO maps to Python INFO (20)

- **Function / Class:** `log_message` / `logTypeToPythonLevel`
- **Scenario:** Log with `LogType.INFO` and verify Python level is INFO.
- **Why it matters:** INFO is the most common log level.
- **Expected behavior:** Python handler records at INFO level.

### Case 6: SUCCESS maps to Python INFO (20)

- **Function / Class:** `log_message` / `logTypeToPythonLevel`
- **Scenario:** Log with `LogType.SUCCESS` and verify Python level is INFO (SUCCESS has no Python equivalent).
- **Why it matters:** SUCCESS is mapped to INFO since Python has no SUCCESS level. Users should expect this mapping.
- **Expected behavior:** Python handler records at INFO level.

### Case 7: WARNING maps to Python WARNING (30)

- **Function / Class:** `log_message` / `logTypeToPythonLevel`
- **Scenario:** Log with `LogType.WARNING` and verify Python level is WARNING.
- **Why it matters:** Warning messages must be visible to Python's default logging configuration (which shows WARNING+).
- **Expected behavior:** Python handler records at WARNING level.

### Case 8: ERROR maps to Python ERROR (40)

- **Function / Class:** `log_message` / `logTypeToPythonLevel`
- **Scenario:** Log with `LogType.ERROR` and verify Python level is ERROR.
- **Why it matters:** Error categorization affects alerting and log analysis.
- **Expected behavior:** Python handler records at ERROR level.

### Case 9: CRITICAL maps to Python CRITICAL (50)

- **Function / Class:** `log_message` / `logTypeToPythonLevel`
- **Scenario:** Log with `LogType.CRITICAL` and verify Python level is CRITICAL.
- **Why it matters:** Critical messages must propagate correctly for alerting.
- **Expected behavior:** Python handler records at CRITICAL level.

---

## Python Convenience Functions

### Case 10: `log_error` routes at Python ERROR level

- **Function / Class:** `log_error` (pybind11 binding)
- **Scenario:** Call `log_error("mod", "fail")` and capture Python output.
- **Why it matters:** Convenience functions must map to the correct LogType internally.
- **Expected behavior:** Message appears at ERROR level in Python logging.

### Case 11: `log_info` routes at Python INFO level

- **Function / Class:** `log_info` (pybind11 binding)
- **Scenario:** Call `log_info("mod", "note")` and capture Python output.
- **Why it matters:** Same as above for INFO.
- **Expected behavior:** Message appears at INFO level.

### Case 12: `log_success` routes at Python INFO level

- **Function / Class:** `log_success` (pybind11 binding)
- **Scenario:** Call `log_success("mod", "ok")` and capture Python output.
- **Why it matters:** SUCCESS maps to INFO in Python. Verify the mapping is correct.
- **Expected behavior:** Message appears at INFO level.

---

## Python Binding — Configuration

### Case 13: `set_log_level` / `get_log_level` round-trip from Python

- **Function / Class:** `Logger.set_log_level` / `Logger.get_log_level` (pybind11 binding)
- **Scenario:** Set a module's level via Python, read it back.
- **Why it matters:** Verifies that the binding correctly passes values to the C++ singleton and reads them back.
- **Expected behavior:** `get_log_level` returns the value set by `set_log_level`.

### Case 14: `disable_colors` / `enable_colors` accessible from Python

- **Function / Class:** `disable_colors` / `enable_colors` (pybind11 binding)
- **Scenario:** Call `disable_colors()` from Python, verify color map entries are empty. Then call `enable_colors()` and verify they are restored.
- **Why it matters:** Python-hosted processes may want to disable terminal colors.
- **Expected behavior:** After `disable_colors`, all color map values are empty strings. After `enable_colors`, they match `DEFAULT_COLOR_MAP`.

### Case 15: Default constant properties accessible from Python

- **Function / Class:** `Logger.DEFAULT_MIN_LOCATION_LENGTH`, `Logger.DEFAULT_MIN_MODULE_LENGTH`
- **Scenario:** Read the static properties from Python.
- **Why it matters:** Python code may reference these defaults for configuration.
- **Expected behavior:** Returns 40 and 20 respectively.
