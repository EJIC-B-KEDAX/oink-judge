# Tests to Add — config

## Config Class Lifecycle

### Case 1: `reloadData` with empty file paths

- **Function / Class:** `Config::reloadData`
- **Scenario:** Call `reloadData()` without ever calling `setConfigFilePath` / `setCredentialsFilePath` (both paths are default-constructed empty paths).
- **Why it matters:** In production, if `reloadData()` is called before paths are set, the constructor will attempt to open an empty path, which should throw. This boundary condition is currently not tested.
- **Expected behavior:** `reloadData()` throws `std::runtime_error` indicating the file could not be opened.

### Case 2: `config()` lazy initialization with empty path

- **Function / Class:** `Config::config`
- **Scenario:** Call `Config::config()` before setting a config file path.
- **Why it matters:** The lazy initialization in `config()` will try to construct a `Config` with a default empty path. This should clearly fail with an appropriate error.
- **Expected behavior:** Throws `std::runtime_error`.

### Case 3: `credentials()` lazy initialization with empty path

- **Function / Class:** `Config::credentials`
- **Scenario:** Call `Config::credentials()` before setting a credentials file path.
- **Why it matters:** Same as Case 2 but for credentials.
- **Expected behavior:** Throws `std::runtime_error`.

### Case 4: `setConfigFilePath` followed by `config()` without `reloadData()`

- **Function / Class:** `Config::setConfigFilePath`, `Config::config`
- **Scenario:** Set a valid config path, call `config()` (lazy init), then call `setConfigFilePath` with a different path, and call `config()` again.
- **Why it matters:** `config()` only creates the instance once (lazy init). After `setConfigFilePath` is called again, the already-created instance is not refreshed. This behavior should be tested to document whether a second `setConfigFilePath` requires `reloadData()`.
- **Expected behavior:** The second `config()` call returns data from the first config file, not the newly set path (until `reloadData()` is called).

---

## `checkPath` and `checkObjectIs*` Functions

### Case 5: `checkPath` with empty path vector

- **Function / Class:** `checkPath`
- **Scenario:** Pass an empty `std::vector<std::string>{}` as the path argument with a JSON object.
- **Why it matters:** When the path is empty, the function checks the type of the root JSON object itself. This edge case is not tested and could behave unexpectedly.
- **Expected behavior:** Returns `true` if the root JSON object matches the specified type, `false` otherwise.

### Case 6: `checkPath` with nested path where intermediate is not an object

- **Function / Class:** `checkPath`
- **Scenario:** Given JSON `{"a": 42}`, call `checkPath(j, {"a", "b"}, json::value_t::string)`.
- **Why it matters:** The intermediate node `"a"` is a number, not an object. Calling `contains()` on a non-object JSON value may throw or behave unexpectedly depending on the nlohmann::json version. This boundary should be tested to verify it returns `false` gracefully.
- **Expected behavior:** Returns `false` without throwing.

### Case 7: `checkObjectIsString` with correct path

- **Function / Class:** `checkObjectIsString`
- **Scenario:** Pass a JSON object with a string value at the given path.
- **Why it matters:** None of the `checkObjectIs*` convenience functions are directly tested. At least the most commonly used ones (`checkObjectIsString`, `checkObjectIsObject`, `checkObjectIsNumber`) should have dedicated test coverage.
- **Expected behavior:** Returns `true`.

### Case 8: `checkObjectIsArray` with correct path

- **Function / Class:** `checkObjectIsArray`
- **Scenario:** Pass a JSON object with an array value at the given path.
- **Why it matters:** Verifies the array-checking convenience function works correctly.
- **Expected behavior:** Returns `true`.

---

## `getDirectoryPath` Edge Cases

### Case 9: `getDirectoryPath` with key pointing to non-string value

- **Function / Class:** `getDirectoryPath`
- **Scenario:** Config JSON has `"directories": {"problems": 123}` (numeric, not string).
- **Why it matters:** The function checks `checkObjectIsString` and should return `nullopt` for non-string values, but this is not tested.
- **Expected behavior:** Returns `std::nullopt`.

---

## `getTiming` Function

### Case 10: `getTiming` returns correct duration for existing key

- **Function / Class:** `getTiming`
- **Scenario:** Config JSON has `"timings": {"short": 1.5}`, call `getTiming("short")`.
- **Why it matters:** `getTiming` is a public API function with zero test coverage. The happy path must be tested.
- **Expected behavior:** Returns `std::optional` containing `std::chrono::duration<double>(1.5)`.

### Case 11: `getTiming` returns nullopt for missing key

- **Function / Class:** `getTiming`
- **Scenario:** Call `getTiming("nonexistent")` when the key does not exist in `"timings"`.
- **Why it matters:** Error path for `getTiming` is not tested.
- **Expected behavior:** Returns `std::nullopt`.

### Case 12: `getTiming` returns nullopt when `timings` section is missing

- **Function / Class:** `getTiming`
- **Scenario:** Config JSON has no `"timings"` key at all.
- **Why it matters:** Tests the case where the entire section is absent.
- **Expected behavior:** Returns `std::nullopt`.

---

## `requireHasValue` Template Function

### Case 13: `requireHasValue` returns value for populated optional

- **Function / Class:** `requireHasValue`
- **Scenario:** Pass `std::optional<int>(42)` to `requireHasValue`.
- **Why it matters:** The happy path of this utility template function is not tested at all. It is used widely (e.g., in `problem_config_utils.cpp`) and should have direct unit tests.
- **Expected behavior:** Returns a const reference to `42`.

### Case 14: `requireHasValue` throws for empty optional

- **Function / Class:** `requireHasValue`
- **Scenario:** Pass `std::nullopt` to `requireHasValue`.
- **Why it matters:** The error path should verify the function throws `std::runtime_error` with the expected message.
- **Expected behavior:** Throws `std::runtime_error` with the default or provided message.

---

## Logger Config Edge Cases

### Case 15: `getLoggerConfig` returns full config with all fields

- **Function / Class:** `getLoggerConfig`
- **Scenario:** Load a good config with all logger fields present and call `getLoggerConfig()`.
- **Why it matters:** `getLoggerConfig()` is tested only for the "missing logger" case. The happy path that returns a fully populated `LoggerConfig` struct is not tested.
- **Expected behavior:** Returns a `LoggerConfig` with all fields populated correctly (output_stream, log_levels, color_map, min_location_length, min_module_length).

### Case 16: `getLoggerOutputStream` defaults to `"stderr"` when `output_stream` key is missing

- **Function / Class:** `getLoggerOutputStream`
- **Scenario:** Config has a `"logger"` object but no `"output_stream"` key inside it.
- **Why it matters:** The function returns `"stderr"` as a default when the key is missing but the logger section exists. This default fallback behavior is not tested.
- **Expected behavior:** Returns `std::optional<std::string>("stderr")`.

### Case 17: `getLoggerColorMap` returns nullopt when `color_map` is missing

- **Function / Class:** `getLoggerColorMap`
- **Scenario:** Config has `"logger"` with `"log_levels"` but no `"color_map"` key.
- **Why it matters:** Tests the partial config case where some optional fields are absent.
- **Expected behavior:** Returns `std::nullopt`.

### Case 18: `getLoggerMinLocationLength` returns nullopt when key is missing

- **Function / Class:** `getLoggerMinLocationLength`
- **Scenario:** Config has `"logger"` but no `"min_location_length"` key.
- **Why it matters:** Tests the fallback when this optional numeric field is absent.
- **Expected behavior:** Returns `std::nullopt`.

### Case 19: `configureLogger` applies all settings

- **Function / Class:** `configureLogger`
- **Scenario:** Construct a `LoggerConfig` with all fields populated and call `configureLogger`.
- **Why it matters:** `configureLogger` is a public API that integrates with the Logger. It is not tested at all, and verifying it correctly delegates to the Logger instance would catch integration regressions.
- **Expected behavior:** Logger instance has updated log levels, min module length, min location length, and color map matching the provided config.

---

## Problem Config Edge Cases

### Case 20: `getProblemConfig` returns nullopt for empty XML document

- **Function / Class:** `getProblemConfig`
- **Scenario:** The problem XML file exists but is empty (no `<problem>` root node).
- **Why it matters:** The `getFullProblemConfig` successfully loads the file, but `getProblemConfig` should return `nullopt` when the `<problem>` child is missing. Currently `getAllTestNames` covers this indirectly but `getProblemConfig` itself has no direct test.
- **Expected behavior:** Returns `std::nullopt`.

### Case 21: `getTestConfig` returns nullopt when problem has no `<tests>` node

- **Function / Class:** `getTestConfig`
- **Scenario:** Problem XML has `<problem>` but no `<tests>` element. Call `getTestConfig("id", "t1")`.
- **Why it matters:** The for-loop iterates over `problem.child("tests").children("test")`, which would iterate zero times if `<tests>` is missing. This should return `nullopt` but is not tested.
- **Expected behavior:** Returns `std::nullopt`.

### Case 22: `getPathToProblemStatements` when statement file does not exist on disk

- **Function / Class:** `getPathToProblemStatements`
- **Scenario:** Problem XML declares a `<statement>` with a `path` attribute that points to a non-existent file.
- **Why it matters:** The function checks `fs::is_regular_file(result)` and should return `nullopt` when the path doesn't resolve to an actual file. This filesystem-dependent branch is not tested.
- **Expected behavior:** Returns `std::nullopt`.

### Case 23: `getPathToProblemStatements` with matching language and type

- **Function / Class:** `getPathToProblemStatements`
- **Scenario:** Problem XML has a `<statement language="en" type="pdf" path="statements/en.pdf"/>` and the file exists on disk.
- **Why it matters:** `getPathToProblemStatements` and `getProblemStatements` have zero test coverage. The happy path should be verified.
- **Expected behavior:** Returns the resolved filesystem path.

### Case 24: `getProblemStatements` happy path

- **Function / Class:** `getProblemStatements`
- **Scenario:** Call with a problem that has a valid statement file on disk.
- **Why it matters:** Zero test coverage for `getProblemStatements`.
- **Expected behavior:** Returns `std::optional<std::string>` containing the file contents.

### Case 25: `getProblemStatements` returns nullopt for non-existent statement

- **Function / Class:** `getProblemStatements`
- **Scenario:** Call with a language/type combination not declared in the XML.
- **Why it matters:** Error path for `getProblemStatements` is untested.
- **Expected behavior:** Returns `std::nullopt`.

---

## Config Credentials

### Case 26: `credentials()` returns correct data

- **Function / Class:** `Config::credentials`
- **Scenario:** After loading a credentials file, call `Config::credentials()` and verify the JSON content.
- **Why it matters:** `Config::credentials()` is never tested directly. All tests only exercise `Config::config()`.
- **Expected behavior:** Returns the JSON data from the credentials file.

### Case 27: `credentials()` with missing credentials file throws

- **Function / Class:** `Config::credentials`
- **Scenario:** Set credentials path to a non-existent file and call `Config::reloadData()`.
- **Why it matters:** Tests only check missing config file, not missing credentials file.
- **Expected behavior:** Throws `std::runtime_error`.
