# Issues — config

## Potential Bugs

### Thread Safety

#### Issue 1: No thread safety on `Config` singleton access

- **Location:** `src/config.cpp:18-30`
- **Description:** `Config::config()` and `Config::credentials()` use lazy initialization of `config_instance` and `credentials_instance` via non-atomic check-then-create. If two threads call `Config::config()` concurrently before the instance is created, both may create a new `Config` object, leading to a data race and undefined behavior. Similarly, `reloadData()` is not synchronized with `config()` / `credentials()`.
- **Suggested fix:** Guard lazy initialization with a `std::mutex` or `std::call_once`, or document that the `Config` class must only be used from a single thread (and ensure this is enforced).

---

### Static Local Cache Bugs in `getFullProblemConfig`

#### Issue 2: Cache always reloads file, defeating the purpose of caching

- **Location:** `src/problem_config_utils.cpp:17-30`
- **Description:** `getFullProblemConfig` uses a `static std::map` as a cache, but every call unconditionally calls `problem_config_cache[problem_id].load_file(...)` — this creates a default-constructed entry on first access via `operator[]`, and then overwrites it with `load_file` on every subsequent call. The cache never actually prevents re-parsing because there is no check for an existing entry before calling `load_file`.
- **Suggested fix:** Check whether `problem_config_cache` already contains the `problem_id` key before loading:
  ```cpp
  auto it = problem_config_cache.find(problem_id);
  if (it != problem_config_cache.end()) {
      return it->second;
  }
  auto& doc = problem_config_cache[problem_id];
  pugi::xml_parse_result result = doc.load_file(problem_config_path.c_str());
  ```

#### Issue 3: `reloadData()` does not invalidate the problem config cache

- **Location:** `src/problem_config_utils.cpp:17` and `src/config.cpp:36-39`
- **Description:** When `Config::reloadData()` is called (e.g., because the problems directory changed), the static `problem_config_cache` in `getFullProblemConfig` still holds stale XML documents from the old config. There is no mechanism to clear this cache when configuration is reloaded.
- **Suggested fix:** Either expose a `clearProblemConfigCache()` function that `reloadData()` can call, or redesign the cache so it doesn't use a function-local static.

#### Issue 4: Static local cache is not thread-safe

- **Location:** `src/problem_config_utils.cpp:17`
- **Description:** The `static std::map` used as the problem config cache is shared mutable state. Concurrent calls to `getFullProblemConfig` with different or identical `problem_id` values can race on the map, leading to undefined behavior.
- **Suggested fix:** Protect access to the cache with a mutex, or document single-threaded usage as a requirement.

---

### `checkPath` Intermediate Node Safety

#### Issue 5: `checkPath` does not verify intermediate nodes are objects

- **Location:** `src/config.cpp:49-57`
- **Description:** When traversing a JSON path, `checkPath` calls `current->contains(key)` on each intermediate node. If an intermediate value is not an object (e.g., `{"a": 42}` with path `{"a", "b"}`), calling `contains()` on a non-object JSON node may throw `nlohmann::json::type_error` depending on the library version. The function is expected to return `false` gracefully in such cases.
- **Suggested fix:** Add a check `if (!current->is_object()) return false;` before the `contains()` call inside the loop.

---

### Type Mismatch in Logger Log Level Functions

#### Issue 6: `getLoggerLogLevel` returns `uint32_t` but stores `int` values

- **Location:** `include/oink_judge/config/logger_utils.h:22` and `src/logger_utils.cpp:76-84`
- **Description:** `getAllLoggerLogLevels()` returns `std::map<std::string, int>` and `LoggerConfig::log_levels` is `std::map<std::string, int>`, but `getLoggerLogLevel()` returns `std::optional<uint32_t>`. The implicit conversion from `int` to `uint32_t` in `return iter->second;` will silently wrap negative values to large unsigned integers.
- **Suggested fix:** Make the return types consistent — either change `getLoggerLogLevel` to return `std::optional<int>`, or change the map value type to `uint32_t` everywhere.

### `getFullProblemConfig` Returns Non-const Reference to Cache

#### Issue 7: Callers can mutate cached XML documents

- **Location:** `include/oink_judge/config/problem_config_utils.h:14` and `src/problem_config_utils.cpp:17`
- **Description:** `getFullProblemConfig` returns `pugi::xml_document&` (non-const reference) to a document stored in the static cache. Any caller can accidentally modify the cached document, causing subsequent calls to receive corrupted data. Since `pugi::xml_document` is non-copyable, returning a const reference is the safest option.
- **Suggested fix:** Change the return type to `const pugi::xml_document&` and update callers accordingly.

---

## Architecture Issues

### Code Duplication in `checkObjectIs*` Functions

#### Issue 8: Three `checkObjectIs*` functions duplicate `checkPath` traversal logic

- **Location:** `src/config.cpp:80-95` (`checkObjectIsNumber`), `src/config.cpp:101-110` (`checkObjectIsNumberInteger`), `src/config.cpp:116-125` (`checkObjectIsPrimitive`)
- **Description:** `checkObjectIsNumber`, `checkObjectIsNumberInteger`, and `checkObjectIsPrimitive` manually duplicate the path-traversal loop from `checkPath` instead of reusing it. Their checks (`.is_number()`, `.is_number_integer()`, `.is_primitive()`) don't map to a single `json::value_t` enum, but a simple refactoring would eliminate the duplication.
- **Suggested fix:** Extract a helper that accepts a predicate:

  ```cpp
  auto checkPathWith(const json& j, const std::vector<std::string>& path,
                     std::function<bool(const json&)> predicate) -> bool;
  ```

  Then implement `checkObjectIsNumber` as `checkPathWith(j, path, [](const json& v) { return v.is_number(); })`, etc.

---

### `configureLogger` Does Not Set Output Stream

#### Issue 9: `configureLogger` has a TODO for output stream configuration

- **Location:** `src/logger_utils.cpp:97`
- **Description:** `configureLogger` reads `output_stream` from the config into `LoggerConfig`, but the actual `configureLogger` function has a `// TODO set output stream here` comment and never uses `config.output_stream`. The output stream field is parsed and stored but has no effect.
- **Suggested fix:** Implement the output stream configuration, or remove the field from `LoggerConfig` if it is not yet needed to avoid misleading callers.
