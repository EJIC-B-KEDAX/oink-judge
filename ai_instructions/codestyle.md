# Code Style & Naming Conventions for oink-judge

Enforced by `.clang-format` and `.clang-tidy` at the project root.

## Formatting (`.clang-format`)

- **Base style:** LLVM
- **Indent:** 4 spaces (no tabs)
- **Column limit:** 130
- **Braces:** Attach (same line)
- **Short statements:** No single-line `if` or `loop` bodies
- **Pointer/reference alignment:** Left (`int* ptr`, `int& ref`)
- **Includes:** Sorted, regrouped (`IncludeBlocks: Regroup`)
- Every file must end with a new line

## Naming Conventions (`.clang-tidy`)


| Entity            | Case                        | Example                                  |
| ----------------- | --------------------------- | ---------------------------------------- |
| Namespace         | `lower_case`                | `oink_judge::config`                     |
| Class / Struct    | `CamelCase`                 | `AsyncServer`, `ConnectionStorage`       |
| Function / Method | `camelBack`                 | `setConfigFilePath()`, `getLogLevel()`   |
| Variable (local)  | `lower_case`                | `start_message_size`, `config_data`      |
| Private member    | `lower_case` + trailing `_` | `out_stream_`, `log_levels_`, `handler_` |
| Constant          | `UPPER_CASE`                | `DEFAULT_MIN_LOCATION_LENGTH`            |

## Namespaces

Nested C++17 syntax: `namespace oink_judge::<module> { ... }`

```cpp
namespace oink_judge::config {
    // ...
} // namespace oink_judge::config
```

Anonymous namespaces for file-local helpers:

```cpp
namespace {
auto safeGetColor(const std::map<std::string, std::string>& color_map, const std::string& key) -> std::string {
    // ...
}
} // namespace
```

## Trailing Return Types

All functions and methods use `auto ... -> ReturnType` syntax. Never use traditional return type placement.

```cpp
// Correct
auto getLogLevel(const std::string& module) const -> int;
auto setOutputStream(std::ostream& out) -> void;
auto asyncConnect(std::string host, short port) -> awaitable<std::shared_ptr<Session>>;

// Wrong
int getLogLevel(const std::string& module) const;
void setOutputStream(std::ostream& out);
```

## Classes

PascalCase name. Private members have trailing `_`. Methods are camelCase with trailing return types.

```cpp
class Logger {
  public:
    static auto instance() -> Logger&;
    auto setOutputStream(std::ostream& out) -> void;
    auto getLogLevel(const std::string& module) const -> int;

    static const uint32_t DEFAULT_MIN_LOCATION_LENGTH = 40;
    static const uint32_t DEFAULT_MIN_MODULE_LENGTH = 20;

  private:
    std::ostream* out_stream_;
    std::map<std::string, int> log_levels_;
    uint32_t min_location_length_;
};
```

## Enums

Unscoped or scoped (`enum class`). Enumerators are `UPPER_CASE`. Specify underlying type.

```cpp
enum LogType : std::uint8_t { DEBUG, INFO, SUCCESS, WARNING, ERROR, CRITICAL };

enum class Type : uint8_t { ADDED, REMOVED, MODIFIED, ATTRIBUTES_CHANGED };
```

## Constants

Class-level constants: `static const` or `constexpr static`, `UPPER_CASE`.

```cpp
class SimpleConnectionHandler {
  public:
    constexpr static auto REGISTERED_NAME = "SimpleConnectionHandler";
};
```

## Type Aliases

`using` declarations to import names, namespace aliases for shortening:

```cpp
using nlohmann::json;
using boost::asio::ip::tcp;
namespace fs = std::filesystem;
```

## Templates

Single uppercase letter for type parameters. Implementations in `.inl` files included at the bottom of the header.

```cpp
// header.h
template <typename T>
auto requireHasValue(const std::optional<T>& opt, const std::string& message) -> const T&;

#include "oink_judge/config/common_utils.inl"

// common_utils.inl
template <typename T>
auto requireHasValue(const std::optional<T>& opt, const std::string& message) -> const T& {
    if (!opt.has_value()) {
        throw std::runtime_error(message);
    }
    return *opt;
}
```

## Headers

- **Guard style:** `#pragma once` (no traditional include guards)
- **No blank line** between `#pragma once` and the first `#include`
- **Include order** (sorted within each group, blank line between groups):
  1. Same-module headers (quoted): `#include "oink_judge/config/config.h"`
  2. Cross-module headers (angle): `#include <oink_judge/logger/logger.h>`
  3. Third-party headers: `#include <gtest/gtest.h>`, `#include <nlohmann/json.hpp>`
  4. Standard library: `#include <string>`, `#include <filesystem>`

## Variables

Local variables: `lower_case`. Use `auto` for complex types. Structured bindings for maps/pairs.

```cpp
auto& io_context = BoostIOContext::instance();
const auto& config_data = Config::config();
auto endpoints = resolver.resolve(host, std::to_string(port));

for (const auto& [key, value] : color_map) {
    // ...
}
```

## Const Placement

`const` before the type:

```cpp
const std::string& name = getName();
const json* current = &data;
auto getColorMap() const -> const std::map<std::string, std::string>&;
```

## File Naming

- Headers: `lower_case.h` in `include/oink_judge/<module>/`
- Sources: `lower_case.cpp` in `src/`
- Template implementations: `lower_case.inl` in `include/oink_judge/<module>/`
- Tests: `test_<feature>.cpp` in `tests/`

## Directory Structure per Module

```
modules/<module>/
├── CMakeLists.txt
├── include/oink_judge/<module>/
│   ├── header.h
│   └── template.inl
├── src/
│   └── implementation.cpp
└── tests/
    ├── CMakeLists.txt
    ├── test_<feature>.cpp
    └── resources/
```

## C++ Standard

C++20. Use modern features: `std::optional`, `std::source_location`, `enum class`, structured bindings, coroutines (`co_await`, `co_return`), concepts where appropriate.
